// ===========================================================================
// Regression tests for the issues listed in doc/AUDIT.md.
//
// Every test here is EXPECTED TO FAIL against the current code and should
// pass once the corresponding audit item is fixed. Test suite names carry the
// audit number (AuditNN_...) so they can be run selectively, e.g.:
//
//     secs-tests --gtest_filter=Audit04*
//
// Conventions:
//  - Tests must compile against today's API. Where a fix introduces a new
//    method (IsAlive, TryGetComponent, RemoveSystem, emplace AddComponent),
//    the method is detected at compile time: the test FAILs while it is
//    missing and exercises its behaviour once it exists.
//  - Scenarios that currently crash the process (null deref, out-of-bounds
//    reads in Debug) run inside EXPECT_EXIT so a crash fails only that test,
//    not the whole suite. Run these in a Debug build: in Release, the
//    out-of-bounds reads may not crash and the tests can pass by accident.
//  - Scenarios that are undefined behaviour today (dangling references)
//    only compare addresses and never dereference stale pointers.
//  - Tests named Guard_* PASS today. They pin down behaviour that a naive fix
//    could break (see the comment on each one).
// ===========================================================================

#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

#include <secs/secs.h>

namespace secs::audit_tests {

// ---------------------------------------------------------------------------
// Components (namespaced to avoid ODR clashes with EngineTests.cpp)
// ---------------------------------------------------------------------------
struct Pos    { float x = 0.f; float y = 0.f; };
struct Vel    { float vx = 0.f; float vy = 0.f; };
struct Hp     { int hp = 100; };
struct Marker { bool set = true; };
struct Unused { int v = 0; };
struct Resource { std::shared_ptr<int> handle; };

// ---------------------------------------------------------------------------
// Systems
// ---------------------------------------------------------------------------
class PosCounterSystem : public secs::TypedEntitySystem<Pos>
{
public:
    int processCount = 0;
    void Process(double, const secs::Entity&, Pos&) override { processCount++; }
};

class MarkerCounterSystem : public secs::TypedEntitySystem<Marker>
{
public:
    int processCount = 0;
    void Process(double, const secs::Entity&, Marker&) override { processCount++; }
};

// Adds a Marker component to every entity it accepts (structural change
// from inside a notification callback).
class MarkOnEnterSystem : public secs::TypedEntitySystem<Pos>
{
public:
    void OnEntityAdded(const secs::Entity& e) override { AddComponent<Marker>(e); }
    void Process(double, const secs::Entity&, Pos&) override {}
};

// Spawns a new entity with a Pos component whenever an Hp entity is removed
// (structural change from inside a removal callback).
class SpawnOnRemoveSystem : public secs::TypedEntitySystem<Hp>
{
public:
    void OnEntityRemoved(const secs::Entity&) override
    {
        auto spawned = GetEntityProcessor()->AddEntity();
        GetEntityProcessor()->AddComponent<Pos>(spawned);
    }
    void Process(double, const secs::Entity&, Hp&) override {}
};

// Spawns many entities with Pos from inside Process and records whether the
// Pos reference it was handed still points at the entity's component.
class SpawnInsideProcessSystem : public secs::TypedEntitySystem<Pos>
{
public:
    bool ran = false;
    bool referenceStable = false;

    void Process(double, const secs::Entity& e, Pos& pos) override
    {
        if (ran) return;
        ran = true;
        const Pos* before = &pos;
        for (int i = 0; i < 256; ++i)
        {
            auto spawned = GetEntityProcessor()->AddEntity();
            GetEntityProcessor()->AddComponent<Pos>(spawned);
        }
        const Pos* after = &GetComponent<Pos>(e);
        referenceStable = (before == after);   // address comparison only
    }
};

// Reads Hp inside OnEntityRemoved (used by the #5 guard test).
class ReadOnRemoveSystem : public secs::TypedEntitySystem<Hp>
{
public:
    int lastSeenHp = -1;
    void OnEntityRemoved(const secs::Entity& e) override { lastSeenHp = GetComponent<Hp>(e).hp; }
    void Process(double, const secs::Entity&, Hp&) override {}
};

class UpdateHookSystem : public secs::System
{
public:
    int preUpdates = 0;
    int postUpdates = 0;
    void PreUpdate(double) override { preUpdates++; }
    void PostUpdate(double) override { postUpdates++; }
};

struct CopyCounter
{
    static inline int copies = 0;
    CopyCounter() = default;
    CopyCounter(const CopyCounter&) { copies++; }
    CopyCounter(CopyCounter&&) noexcept {}
};

class SystemWithArg : public secs::System
{
public:
    explicit SystemWithArg(CopyCounter c) : m_arg(std::move(c)) {}
private:
    CopyCounter m_arg;
};

// ---------------------------------------------------------------------------
// Compile-time detection of APIs that the audit fixes introduce
// ---------------------------------------------------------------------------
template <typename, typename = void>
struct HasIsAlive : std::false_type {};
template <typename T>
struct HasIsAlive<T, std::void_t<decltype(std::declval<T&>().IsAlive(std::declval<secs::Entity>()))>>
    : std::true_type {};

template <typename, typename = void>
struct HasTryGetComponent : std::false_type {};
template <typename T>
struct HasTryGetComponent<T, std::void_t<decltype(std::declval<T&>().template TryGetComponent<Pos>(std::declval<secs::Entity>()))>>
    : std::true_type {};

template <typename, typename = void>
struct HasRemoveSystem : std::false_type {};
template <typename T>
struct HasRemoveSystem<T, std::void_t<decltype(std::declval<T&>().RemoveSystem(std::declval<secs::System::Ptr>()))>>
    : std::true_type {};

template <typename, typename = void>
struct HasEmplaceAddComponent : std::false_type {};
template <typename T>
struct HasEmplaceAddComponent<T, std::void_t<decltype(std::declval<T&>().template AddComponent<Pos>(std::declval<secs::Entity>(), 1.f, 2.f))>>
    : std::true_type {};

template <typename, typename = void>
struct HasConstIsEnabled : std::false_type {};
template <typename T>
struct HasConstIsEnabled<T, std::void_t<decltype(std::declval<const T&>().IsEnabled())>>
    : std::true_type {};

// ---------------------------------------------------------------------------
// Fixtures
// ---------------------------------------------------------------------------
class AuditFixture : public testing::Test
{
protected:
    secs::Engine engine;
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

class AuditBackendFixture : public testing::TestWithParam<secs::StorageMode>
{
protected:
    secs::Engine engine{ GetParam() };
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

std::string BackendName(const testing::TestParamInfo<secs::StorageMode>& info)
{
    return info.param == secs::StorageMode::Vector ? "Vector" : "SparseSet";
}

// ===========================================================================
// #1 — TagEntity wipes every existing tag
// ===========================================================================
using Audit01_Tags = AuditFixture;

TEST_F(Audit01_Tags, IntroducingNewTagKeepsExistingTags)
{
    secs::Entity a = proc.AddEntity();
    secs::Entity b = proc.AddEntity();
    proc.TagEntity(a, "player");
    proc.TagEntity(b, "enemy");

    auto players = proc.GetAllEntitiesWithTag("player");
    ASSERT_EQ(players.size(), 1u);
    EXPECT_EQ(players[0], a);
}

TEST_F(Audit01_Tags, TaggingSameEntityTwiceDoesNotDuplicate)
{
    secs::Entity a = proc.AddEntity();
    proc.TagEntity(a, "player");
    proc.TagEntity(a, "player");

    EXPECT_EQ(proc.GetAllEntitiesWithTag("player").size(), 1u);
}

TEST_F(Audit01_Tags, RemovedEntityIsDroppedFromTags)
{
    secs::Entity a = proc.AddEntity();
    proc.TagEntity(a, "player");
    proc.ForceApplyChanges();

    proc.RemoveEntity(a);
    proc.ForceApplyChanges();

    EXPECT_TRUE(proc.GetAllEntitiesWithTag("player").empty());
}

// ===========================================================================
// #2 — Structural changes from callbacks during ApplyChanges are lost
// ===========================================================================
using Audit02_Reentrancy = AuditFixture;

TEST_F(Audit02_Reentrancy, ComponentAddedInOnEntityAddedIsApplied)
{
    engine.CreateSystem<MarkOnEnterSystem>();
    auto counter = engine.CreateSystem<MarkerCounterSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Pos>(e);

    engine.Step(0.0);
    engine.Step(0.0);

    EXPECT_TRUE(proc.HasComponent<Marker>(e));
    EXPECT_GT(counter->processCount, 0);
}

TEST_F(Audit02_Reentrancy, EntitySpawnedInOnEntityRemovedReachesSystems)
{
    engine.CreateSystem<SpawnOnRemoveSystem>();
    auto posCounter = engine.CreateSystem<PosCounterSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Hp>(e);
    engine.Step(0.0);

    proc.RemoveEntity(e);
    engine.Step(0.0);   // removal processed -> callback spawns an entity with Pos
    engine.Step(0.0);

    EXPECT_GT(posCounter->processCount, 0);
}

// ===========================================================================
// #3 — Component references are invalidated by later additions
// ===========================================================================
class Audit03_ReferenceStability : public AuditBackendFixture {};

INSTANTIATE_TEST_SUITE_P(Backends, Audit03_ReferenceStability,
    testing::Values(secs::StorageMode::Vector, secs::StorageMode::SparseSet), BackendName);

TEST_P(Audit03_ReferenceStability, AddingToSamePoolDoesNotMoveExistingComponent)
{
    secs::Entity first = proc.AddEntity();
    const Pos* addr = &proc.AddComponent<Pos>(first);

    for (int i = 0; i < 1000; ++i)
    {
        proc.AddComponent<Pos>(proc.AddEntity());
    }

    EXPECT_EQ(addr, &proc.GetComponent<Pos>(first));
}

TEST_P(Audit03_ReferenceStability, ProcessArgumentSurvivesSpawningInsideProcess)
{
    auto sys = engine.CreateSystem<SpawnInsideProcessSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Pos>(e);
    engine.Step(0.0);

    ASSERT_TRUE(sys->ran);
    EXPECT_TRUE(sys->referenceStable);
}

// ===========================================================================
// #4 / #17 — Deferred flags: double add resets data, add+remove is lost,
//            HasComponent disagrees with GetComponent within a frame
// ===========================================================================
using Audit04_SameFrameEdits = AuditFixture;

TEST_F(Audit04_SameFrameEdits, AddOrRetrieveTwiceInSameFrameKeepsData)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Hp>(e).hp = 50;
    proc.AddOrRetrieveComponent<Hp>(e);

    EXPECT_EQ(proc.GetComponent<Hp>(e).hp, 50);
}

TEST_F(Audit04_SameFrameEdits, AddThenRemoveInSameFrameLeavesNoComponent)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Hp>(e);
    proc.RemoveComponent<Hp>(e);
    proc.ForceApplyChanges();

    EXPECT_FALSE(proc.HasComponent<Hp>(e));
}

TEST_F(Audit04_SameFrameEdits, HasComponentIsTrueRightAfterAddComponent)   // #17
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Hp>(e);

    EXPECT_TRUE(proc.HasComponent<Hp>(e));
}

TEST_F(Audit04_SameFrameEdits, HasComponentIsFalseRightAfterRemoveComponent)   // #17
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Hp>(e);
    proc.ForceApplyChanges();

    proc.RemoveComponent<Hp>(e);

    EXPECT_FALSE(proc.HasComponent<Hp>(e));
}

// ===========================================================================
// #5 — Components are never freed
// ===========================================================================
class Audit05_ComponentFreeing : public AuditBackendFixture {};

INSTANTIATE_TEST_SUITE_P(Backends, Audit05_ComponentFreeing,
    testing::Values(secs::StorageMode::Vector, secs::StorageMode::SparseSet), BackendName);

TEST_P(Audit05_ComponentFreeing, RemoveComponentReleasesResources)
{
    auto handle = std::make_shared<int>(7);
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Resource>(e).handle = handle;
    proc.ForceApplyChanges();
    ASSERT_EQ(handle.use_count(), 2);

    proc.RemoveComponent<Resource>(e);
    proc.ForceApplyChanges();

    EXPECT_EQ(handle.use_count(), 1);
}

TEST_P(Audit05_ComponentFreeing, RemoveEntityReleasesResources)
{
    auto handle = std::make_shared<int>(7);
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Resource>(e).handle = handle;
    proc.ForceApplyChanges();
    ASSERT_EQ(handle.use_count(), 2);

    proc.RemoveEntity(e);
    proc.ForceApplyChanges();

    EXPECT_EQ(handle.use_count(), 1);
}

// Guard (passes today): SPECS §5 says components are freed *after* systems
// are notified, so OnEntityRemoved must still be able to read them. A fix
// for #5 that frees before notifying would break this.
TEST_P(Audit05_ComponentFreeing, Guard_ComponentReadableInOnEntityRemoved)
{
    auto sys = engine.CreateSystem<ReadOnRemoveSystem>();
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Hp>(e).hp = 42;
    engine.Step(0.0);

    proc.RemoveEntity(e);
    engine.Step(0.0);

    EXPECT_EQ(sys->lastSeenHp, 42);
}

// ===========================================================================
// #6 — No generation counter: stale handles alias recycled entities
// ===========================================================================
using Audit06_Generations = AuditFixture;

TEST_F(Audit06_Generations, RecycledEntityIsNotEqualToStaleHandle)
{
    secs::Entity stale = proc.AddEntity();
    proc.ForceApplyChanges();
    proc.RemoveEntity(stale);
    proc.ForceApplyChanges();

    secs::Entity fresh = proc.AddEntity();

    EXPECT_FALSE(stale == fresh);
}

template <typename Processor>
void CheckIsAlive(Processor& proc)
{
    if constexpr (HasIsAlive<Processor>::value)
    {
        secs::Entity stale = proc.AddEntity();
        proc.ForceApplyChanges();
        proc.RemoveEntity(stale);
        proc.ForceApplyChanges();
        secs::Entity fresh = proc.AddEntity();
        proc.ForceApplyChanges();

        EXPECT_FALSE(proc.IsAlive(stale));
        EXPECT_TRUE(proc.IsAlive(fresh));
    }
    else
    {
        FAIL() << "EntityProcessor::IsAlive(Entity) does not exist yet";
    }
}

TEST_F(Audit06_Generations, IsAliveDetectsStaleHandle)
{
    CheckIsAlive(proc);
}

// ===========================================================================
// #7 — Systems added after entities exist never see them
// ===========================================================================
using Audit07_LateSystems = AuditFixture;

TEST_F(Audit07_LateSystems, SystemCreatedAfterEntitiesProcessesThem)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Pos>(e);
    engine.Step(0.0);

    auto sys = engine.CreateSystem<PosCounterSystem>();
    engine.Step(0.0);

    EXPECT_EQ(sys->processCount, 1);
}

// ===========================================================================
// #8 — SetSystemGroup dereferences null when the group wasn't activated
//      (runs in a child process because it crashes today)
// ===========================================================================
void RunSetSystemGroupWithoutActivate()
{
    secs::Engine engine;
    auto sys = engine.CreateSystem<PosCounterSystem>();
    engine.SetSystemGroup(sys, 3);
    engine.DisableGroup(3);

    secs::Entity e = engine.GetEntityProcessor().AddEntity();
    engine.GetEntityProcessor().AddComponent<Pos>(e);
    engine.Step(0.0);

    // 0 = fixed; 2 = no crash but the group's system still ran.
    // (A crash shows up as a non-zero SEH/abort status, or 1 when gtest
    // catches the access violation in the child.)
    std::exit(sys->processCount == 0 ? 0 : 2);
}

TEST(Audit08_SystemGroupsDeathTest, SetSystemGroupWithoutActivateDoesNotCrash)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_EXIT(RunSetSystemGroupWithoutActivate(), testing::ExitedWithCode(0), "");
}

// ===========================================================================
// #9 — Out-of-bounds reads and entity-count drift
// ===========================================================================
using Audit09_Bounds = AuditFixture;

TEST_F(Audit09_Bounds, RemovingEntityTwiceDoesNotDriftCount)
{
    secs::Entity e = proc.AddEntity();
    proc.ForceApplyChanges();

    proc.RemoveEntity(e);
    proc.RemoveEntity(e);
    proc.ForceApplyChanges();

    EXPECT_EQ(proc.GetNumEntities(), 0);
}

void RunRemoveMissingComponent()
{
    secs::Engine engine;
    auto& proc = engine.GetEntityProcessor();
    secs::Entity e = proc.AddEntity();
    proc.ForceApplyChanges();

    proc.RemoveComponent<Unused>(e);   // entity never had Unused
    proc.ForceApplyChanges();

    std::exit(proc.HasComponent<Unused>(e) ? 1 : 0);
}

void RunHasComponentOnUnknownEntity()
{
    secs::Engine engine;
    bool has = engine.HasComponent<Pos>(secs::Entity(1000));   // never created
    std::exit(has ? 1 : 0);
}

TEST(Audit09_BoundsDeathTest, RemoveComponentThatWasNeverAddedIsSafeNoOp)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_EXIT(RunRemoveMissingComponent(), testing::ExitedWithCode(0), "");
}

TEST(Audit09_BoundsDeathTest, HasComponentOnUnknownEntityReturnsFalse)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_EXIT(RunHasComponentOnUnknownEntity(), testing::ExitedWithCode(0), "");
}

template <typename Processor>
void CheckTryGetComponent(Processor& proc)
{
    if constexpr (HasTryGetComponent<Processor>::value)
    {
        secs::Entity with = proc.AddEntity();
        secs::Entity without = proc.AddEntity();
        proc.template AddComponent<Pos>(with);
        proc.ForceApplyChanges();

        EXPECT_NE(proc.template TryGetComponent<Pos>(with), nullptr);
        EXPECT_EQ(proc.template TryGetComponent<Pos>(without), nullptr);
    }
    else
    {
        FAIL() << "EntityProcessor::TryGetComponent<T>(Entity) does not exist yet";
    }
}

TEST_F(Audit09_Bounds, TryGetComponentReturnsNullForMissingComponent)
{
    CheckTryGetComponent(proc);
}

// ===========================================================================
// #15 — Two conflicting eid_t types
// ===========================================================================
TEST(Audit15_EntityIdType, SingleEntityIdType)
{
    EXPECT_TRUE((std::is_same_v<secs::eid_t, secs::Entity::eid_t>))
        << "secs::eid_t and secs::Entity::eid_t differ (uint32_t vs uint64_t)";
}

// ===========================================================================
// #16 — Dead API: DeactivateEntity is a no-op
// (If the fix is to delete DeactivateEntity instead, delete this test too.)
// ===========================================================================
using Audit16_DeadCode = AuditFixture;

TEST_F(Audit16_DeadCode, DeactivatedEntityIsNotProcessed)
{
    auto sys = engine.CreateSystem<PosCounterSystem>();
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Pos>(e);
    engine.Step(0.0);
    ASSERT_EQ(sys->processCount, 1);

    proc.DeactivateEntity(e);
    engine.Step(0.0);

    EXPECT_EQ(sys->processCount, 1);
}

// ===========================================================================
// #18 — Ownership and lifetime
// ===========================================================================
TEST(Audit18_Ownership, EngineIsNotCopyConstructible)
{
    EXPECT_FALSE(std::is_copy_constructible_v<secs::Engine>)
        << "A copied Engine's processor/managers would reference the original";
}

template <typename EngineT>
void CheckRemoveSystem(EngineT& engine)
{
    if constexpr (HasRemoveSystem<EngineT>::value)
    {
        auto& proc = engine.GetEntityProcessor();
        auto sys = engine.template CreateSystem<PosCounterSystem>();
        secs::Entity e = proc.AddEntity();
        proc.template AddComponent<Pos>(e);
        engine.Step(0.0);
        ASSERT_EQ(sys->processCount, 1);

        engine.RemoveSystem(sys);
        engine.Step(0.0);

        EXPECT_EQ(sys->processCount, 1);
    }
    else
    {
        FAIL() << "Engine::RemoveSystem(System::Ptr) does not exist yet";
    }
}

TEST_F(AuditFixture, Audit18_RemoveSystemStopsProcessing)
{
    CheckRemoveSystem(engine);
}

// ===========================================================================
// #19 — Global component-type registry starts at index 1 (wastes bit 0)
// (child process so that this is the first component type registered)
// ===========================================================================
void RunFirstComponentIndex()
{
    struct FirstEverComponent {};
    std::exit(static_cast<int>(secs::ComponentTraits::GetIndexForComponentType<FirstEverComponent>()));
}

TEST(Audit19_TypeRegistryDeathTest, FirstComponentTypeGetsIndexZero)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_EXIT(RunFirstComponentIndex(), testing::ExitedWithCode(0), "");
}

// ===========================================================================
// #20 — Smaller API issues
// ===========================================================================
using Audit20_Api = AuditFixture;

TEST_F(Audit20_Api, CreateSystemForwardsArguments)
{
    CopyCounter::copies = 0;
    engine.CreateSystem<SystemWithArg>(CopyCounter{});

    EXPECT_EQ(CopyCounter::copies, 0) << "rvalue argument was copied instead of moved";
}

TEST_F(Audit20_Api, DisabledSystemDoesNotRunUpdateHooks)
{
    auto sys = engine.CreateSystem<UpdateHookSystem>();
    sys->Disable();

    engine.Step(0.0);

    EXPECT_EQ(sys->preUpdates, 0);
    EXPECT_EQ(sys->postUpdates, 0);
}

TEST(Audit20_ApiConst, SystemQueriesAreConst)
{
    EXPECT_TRUE(HasConstIsEnabled<secs::System>::value)
        << "System::IsEnabled() should be callable on a const System";
}

template <typename Processor>
void CheckEmplaceAddComponent(Processor& proc)
{
    if constexpr (HasEmplaceAddComponent<Processor>::value)
    {
        secs::Entity e = proc.AddEntity();
        Pos& p = proc.template AddComponent<Pos>(e, 3.f, 4.f);
        EXPECT_FLOAT_EQ(p.x, 3.f);
        EXPECT_FLOAT_EQ(p.y, 4.f);
    }
    else
    {
        FAIL() << "EntityProcessor::AddComponent<T>(Entity, Args&&...) does not exist yet";
    }
}

TEST_F(Audit20_Api, AddComponentAcceptsConstructorArguments)
{
    CheckEmplaceAddComponent(proc);
}

} // namespace secs::audit_tests
