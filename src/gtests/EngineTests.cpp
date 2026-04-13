#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <secs/secs.h>

// ---------------------------------------------------------------------------
// Test components
// ---------------------------------------------------------------------------
struct Position { float x = 0.f; float y = 0.f; };
struct Velocity { float vx = 0.f; float vy = 0.f; };
struct Health   { int hp = 100; };

namespace secs::tests {

// ===========================================================================
// Entity tests
// ===========================================================================
class EntityTests : public testing::Test {};

TEST_F(EntityTests, DefaultEntityIsInvalid)
{
    secs::Entity e;
    EXPECT_FALSE(e.IsValid());
}

TEST_F(EntityTests, EntityWithIdIsValid)
{
    secs::Entity e(0);
    EXPECT_TRUE(e.IsValid());
}

TEST_F(EntityTests, InvalidateEntity)
{
    secs::Entity e(42);
    e.Invalidate();
    EXPECT_FALSE(e.IsValid());
}

TEST_F(EntityTests, EqualityHoldsBetweenSameId)
{
    secs::Entity a(7), b(7);
    EXPECT_EQ(a, b);
}

TEST_F(EntityTests, LessThanOrdersByEID)
{
    secs::Entity lo(1), hi(2);
    EXPECT_LT(lo, hi);
}

// ===========================================================================
// Entity lifecycle tests (using Engine)
// ===========================================================================
class EntityLifecycleTests : public testing::Test
{
protected:
    secs::Engine engine;
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

TEST_F(EntityLifecycleTests, AddedEntityIsValid)
{
    secs::Entity e = proc.AddEntity();
    EXPECT_TRUE(e.IsValid());
}

TEST_F(EntityLifecycleTests, EntityCountIncreasesOnAdd)
{
    proc.AddEntity();
    proc.AddEntity();
    EXPECT_EQ(proc.GetNumEntities(), 2);
}

TEST_F(EntityLifecycleTests, EntityCountDecreasesOnRemove)
{
    secs::Entity e = proc.AddEntity();
    proc.RemoveEntity(e);
    EXPECT_EQ(proc.GetNumEntities(), 0);
}

TEST_F(EntityLifecycleTests, ValidEntitiesReflectsAddAfterApply)
{
    proc.AddEntity();
    proc.AddEntity();
    proc.ForceApplyChanges();
    EXPECT_EQ(static_cast<int>(proc.ValidEntities().size()), 2);
}

TEST_F(EntityLifecycleTests, ValidEntitiesReflectsRemoveAfterApply)
{
    secs::Entity e = proc.AddEntity();
    proc.ForceApplyChanges();
    proc.RemoveEntity(e);
    proc.ForceApplyChanges();
    EXPECT_EQ(static_cast<int>(proc.ValidEntities().size()), 0);
}

TEST_F(EntityLifecycleTests, TwoEntitiesHaveDistinctIDs)
{
    secs::Entity a = proc.AddEntity();
    secs::Entity b = proc.AddEntity();
    EXPECT_NE(a.GetEID(), b.GetEID());
}

// ===========================================================================
// Component tests
// ===========================================================================
class ComponentTests : public testing::Test
{
protected:
    secs::Engine engine;
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

TEST_F(ComponentTests, HasComponentReturnsFalseBeforeAdd)
{
    secs::Entity e = proc.AddEntity();
    EXPECT_FALSE(proc.HasComponent<Position>(e));
}

TEST_F(ComponentTests, HasComponentReturnsTrueAfterAddAndApply)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.ForceApplyChanges();
    EXPECT_TRUE(proc.HasComponent<Position>(e));
}

TEST_F(ComponentTests, ComponentDataCanBeWrittenAndRead)
{
    secs::Entity e = proc.AddEntity();
    Position& p = proc.AddComponent<Position>(e);
    p.x = 3.f;
    p.y = 7.f;
    proc.ForceApplyChanges();

    Position& stored = proc.GetComponent<Position>(e);
    EXPECT_FLOAT_EQ(stored.x, 3.f);
    EXPECT_FLOAT_EQ(stored.y, 7.f);
}

TEST_F(ComponentTests, MultipleComponentsCanCoexistOnEntity)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.AddComponent<Velocity>(e);
    proc.ForceApplyChanges();

    EXPECT_TRUE(proc.HasComponent<Position>(e));
    EXPECT_TRUE(proc.HasComponent<Velocity>(e));
}

TEST_F(ComponentTests, HasComponentReturnsFalseAfterRemoveAndApply)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.ForceApplyChanges();
    proc.RemoveComponent<Position>(e);
    proc.ForceApplyChanges();

    EXPECT_FALSE(proc.HasComponent<Position>(e));
}

TEST_F(ComponentTests, EngineHelperHasComponentMatchesProcessor)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Health>(e);
    proc.ForceApplyChanges();

    EXPECT_TRUE(engine.HasComponent<Health>(e));
    EXPECT_FALSE(engine.HasComponent<Position>(e));
}

TEST_F(ComponentTests, EngineAddAndGetComponentRoundtrip)
{
    secs::Entity e = proc.AddEntity();
    engine.AddComponent<Health>(e);
    proc.ForceApplyChanges();

    engine.GetComponent<Health>(e).hp = 42;
    EXPECT_EQ(engine.GetComponent<Health>(e).hp, 42);
}

// ===========================================================================
// System tests
// ===========================================================================

// A simple system that counts how many times it processes entities and
// modifies the Position component.
class MoveSystem : public secs::TypedEntitySystem<Position, Velocity>
{
public:
    int processCount = 0;

    void Process(double /*delta*/, const secs::Entity& /*e*/,
                 Position& pos, Velocity& vel) override
    {
        pos.x += vel.vx;
        pos.y += vel.vy;
        processCount++;
    }
};

// A system that only handles Position (no Velocity).
class PositionOnlySystem : public secs::TypedEntitySystem<Position>
{
public:
    int processCount = 0;

    void Process(double /*delta*/, const secs::Entity& /*e*/,
                 Position& /*pos*/) override
    {
        processCount++;
    }
};

class SystemTests : public testing::Test
{
protected:
    secs::Engine engine;
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

TEST_F(SystemTests, SystemProcessesEntityWithMatchingComponents)
{
    auto sys = engine.CreateSystem<MoveSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    Velocity& vel = proc.AddComponent<Velocity>(e);
    vel.vx = 1.f; vel.vy = 2.f;

    engine.Step(0.0);   // ApplyChanges + Step

    EXPECT_EQ(sys->processCount, 1);
}

TEST_F(SystemTests, SystemDoesNotProcessEntityMissingComponent)
{
    auto sys = engine.CreateSystem<MoveSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    // No Velocity — MoveSystem requires both

    engine.Step(0.0);

    EXPECT_EQ(sys->processCount, 0);
}

TEST_F(SystemTests, SystemModifiesComponentValueCorrectly)
{
    engine.CreateSystem<MoveSystem>();

    secs::Entity e = proc.AddEntity();
    Position& pos = proc.AddComponent<Position>(e);
    Velocity& vel = proc.AddComponent<Velocity>(e);
    pos.x = 0.f; pos.y = 0.f;
    vel.vx = 5.f; vel.vy = 3.f;

    engine.Step(0.0);

    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(e).x, 5.f);
    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(e).y, 3.f);
}

TEST_F(SystemTests, SystemProcessesMultipleEntities)
{
    auto sys = engine.CreateSystem<MoveSystem>();

    for (int i = 0; i < 5; ++i)
    {
        secs::Entity e = proc.AddEntity();
        proc.AddComponent<Position>(e);
        proc.AddComponent<Velocity>(e);
    }

    engine.Step(0.0);

    EXPECT_EQ(sys->processCount, 5);
}

TEST_F(SystemTests, SystemStopsProcessingEntityAfterComponentRemoved)
{
    auto sys = engine.CreateSystem<MoveSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.AddComponent<Velocity>(e);
    engine.Step(0.0);   // entity accepted by system
    EXPECT_EQ(sys->processCount, 1);

    proc.RemoveComponent<Velocity>(e);
    engine.Step(0.0);   // entity should be evicted from system

    EXPECT_EQ(sys->processCount, 1);  // no new processing
}

TEST_F(SystemTests, TwoSystemsWithDifferentRequirementsRunIndependently)
{
    auto moveSys = engine.CreateSystem<MoveSystem>();
    auto posSys  = engine.CreateSystem<PositionOnlySystem>();

    // Entity with both components — both systems should process it
    secs::Entity e1 = proc.AddEntity();
    proc.AddComponent<Position>(e1);
    proc.AddComponent<Velocity>(e1);

    // Entity with only Position — only PositionOnlySystem should process it
    secs::Entity e2 = proc.AddEntity();
    proc.AddComponent<Position>(e2);

    engine.Step(0.0);

    EXPECT_EQ(moveSys->processCount,  1);
    EXPECT_EQ(posSys->processCount,   2);
}

TEST_F(SystemTests, EntityRemovedFromEngineIsNoLongerProcessed)
{
    auto sys = engine.CreateSystem<MoveSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.AddComponent<Velocity>(e);
    engine.Step(0.0);
    EXPECT_EQ(sys->processCount, 1);

    proc.RemoveEntity(e);
    engine.Step(0.0);

    EXPECT_EQ(sys->processCount, 1);
}

// ===========================================================================
// Tag tests
// ===========================================================================
class TagTests : public testing::Test
{
protected:
    secs::Engine engine;
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

TEST_F(TagTests, TaggedEntityIsRetrievableByTag)
{
    secs::Entity e = proc.AddEntity();
    proc.TagEntity(e, "player");

    auto results = proc.GetAllEntitiesWithTag("player");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].GetEID(), e.GetEID());
}

TEST_F(TagTests, UnknownTagReturnsEmptyVector)
{
    auto results = proc.GetAllEntitiesWithTag("nonexistent");
    EXPECT_TRUE(results.empty());
}

// ===========================================================================
// Visit tests
// ===========================================================================
class VisitTests : public testing::Test
{
protected:
    secs::Engine engine;
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

TEST_F(VisitTests, VisitCallsVisitorForEachValidEntity)
{
    proc.AddEntity();
    proc.AddEntity();
    proc.ForceApplyChanges();

    int count = 0;
    engine.Visit([&count](const secs::Entity&) { count++; });

    EXPECT_EQ(count, 2);
}

TEST_F(VisitTests, VisitDoesNotVisitRemovedEntities)
{
    secs::Entity e1 = proc.AddEntity();
    proc.AddEntity();
    proc.ForceApplyChanges();

    proc.RemoveEntity(e1);
    proc.ForceApplyChanges();

    int count = 0;
    engine.Visit([&count](const secs::Entity&) { count++; });

    EXPECT_EQ(count, 1);
}

// ===========================================================================
// Storage mode tests
// ===========================================================================

// Parameterised fixture so we run the same assertions against both backends.
class StorageModeTests : public testing::TestWithParam<secs::StorageMode>
{
protected:
    secs::Engine engine{ GetParam() };
    secs::EntityProcessor& proc{ engine.GetEntityProcessor() };
};

INSTANTIATE_TEST_SUITE_P(
    BothBackends,
    StorageModeTests,
    testing::Values(secs::StorageMode::Vector, secs::StorageMode::SparseSet),
    [](const testing::TestParamInfo<secs::StorageMode>& info) {
        return info.param == secs::StorageMode::Vector ? "Vector" : "SparseSet";
    }
);

TEST_P(StorageModeTests, AllocAndGetComponent_RoundTrip)
{
    secs::Entity e = proc.AddEntity();
    Position& p = proc.AddComponent<Position>(e);
    p.x = 1.f; p.y = 2.f;
    proc.ForceApplyChanges();

    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(e).x, 1.f);
    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(e).y, 2.f);
}

TEST_P(StorageModeTests, HasComponent_TrueAfterAdd_FalseAfterRemove)
{
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Health>(e);
    proc.ForceApplyChanges();
    EXPECT_TRUE(proc.HasComponent<Health>(e));

    proc.RemoveComponent<Health>(e);
    proc.ForceApplyChanges();
    EXPECT_FALSE(proc.HasComponent<Health>(e));
}

TEST_P(StorageModeTests, MultipleEntities_ComponentsAreIndependent)
{
    secs::Entity a = proc.AddEntity();
    secs::Entity b = proc.AddEntity();
    proc.AddComponent<Position>(a).x = 10.f;
    proc.AddComponent<Position>(b).x = 20.f;
    proc.ForceApplyChanges();

    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(a).x, 10.f);
    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(b).x, 20.f);
}

TEST_P(StorageModeTests, SystemProcessesEntityInBothModes)
{
    auto sys = engine.CreateSystem<MoveSystem>();

    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.AddComponent<Velocity>(e).vx = 3.f;

    engine.Step(0.0);

    EXPECT_EQ(sys->processCount, 1);
    EXPECT_FLOAT_EQ(proc.GetComponent<Position>(e).x, 3.f);
}

} // namespace secs::tests


