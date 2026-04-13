#include <benchmark/benchmark.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include <secs/secs.h>

// ---------------------------------------------------------------------------
// Test components
// ---------------------------------------------------------------------------
struct Pos  { float x = 0.f, y = 0.f, z = 0.f; };
struct Vel  { float vx = 0.f, vy = 0.f, vz = 0.f; };
struct Mass { float value = 1.f; };

// ---------------------------------------------------------------------------
// Helper — build an Engine with N entities, each holding Pos + Vel.
// ForceApplyChanges() is called once so the state is fully committed.
// Returns the list of created entities for callers that need them.
// ---------------------------------------------------------------------------
static std::vector<secs::Entity> BuildEngine(secs::Engine& engine, int n)
{
    auto& proc = engine.GetEntityProcessor();
    std::vector<secs::Entity> entities;
    entities.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        secs::Entity e = proc.AddEntity();
        proc.AddComponent<Pos>(e);
        proc.AddComponent<Vel>(e).vx = static_cast<float>(i);
        entities.push_back(e);
    }
    proc.ForceApplyChanges();
    return entities;
}

// ---------------------------------------------------------------------------
// 1. Sequential iteration
//    Simulates the inner loop of EntitySystem::Step — read every component
//    in entity-insertion order (best-case spatial locality).
// ---------------------------------------------------------------------------
static void BM_Iteration(benchmark::State& state, secs::StorageMode mode)
{
    const int n = static_cast<int>(state.range(0));
    secs::Engine engine(mode);
    auto entities = BuildEngine(engine, n);
    auto& proc = engine.GetEntityProcessor();

    for (auto _ : state)
    {
        float acc = 0.f;
        for (const auto& e : entities)
        {
            auto& p = proc.GetComponent<Pos>(e);
            benchmark::DoNotOptimize(p);
            acc += p.x;
        }
        benchmark::DoNotOptimize(acc);
    }
    state.SetItemsProcessed(state.iterations() * n);
    state.counters["mem_bytes"] = benchmark::Counter(
        static_cast<double>(proc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
        benchmark::Counter::kDefaults,
        benchmark::Counter::OneK::kIs1024);
}
BENCHMARK_CAPTURE(BM_Iteration, Vector,    secs::StorageMode::Vector)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_Iteration, SparseSet, secs::StorageMode::SparseSet)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);

// ---------------------------------------------------------------------------
// 2. Random-access GetComponent
//    Shuffles entity IDs, then reads components in random order.
//    Worst-case cache behaviour — highlights the impact of dense packing.
// ---------------------------------------------------------------------------
static void BM_RandomAccess(benchmark::State& state, secs::StorageMode mode)
{
    const int n = static_cast<int>(state.range(0));
    secs::Engine engine(mode);
    auto entities = BuildEngine(engine, n);
    auto& proc = engine.GetEntityProcessor();

    // Shuffle once outside the timed loop so we measure access cost, not shuffle cost.
    std::mt19937 rng(42);
    std::shuffle(entities.begin(), entities.end(), rng);

    for (auto _ : state)
    {
        float acc = 0.f;
        for (const auto& e : entities)
        {
            auto& p = proc.GetComponent<Pos>(e);
            benchmark::DoNotOptimize(p);
            acc += p.x;
        }
        benchmark::DoNotOptimize(acc);
    }
    state.SetItemsProcessed(state.iterations() * n);
    state.counters["mem_bytes"] = benchmark::Counter(
        static_cast<double>(proc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
        benchmark::Counter::kDefaults,
        benchmark::Counter::OneK::kIs1024);
}
BENCHMARK_CAPTURE(BM_RandomAccess, Vector,    secs::StorageMode::Vector)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_RandomAccess, SparseSet, secs::StorageMode::SparseSet)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);

// ---------------------------------------------------------------------------
// 3. AllocComponent burst
//    Repeatedly creates N entities and adds a component to each.
//    Measures the cost of AddEntity + AddComponent + ForceApplyChanges.
//    The engine is rebuilt from scratch each outer iteration.
// ---------------------------------------------------------------------------
static void BM_AllocBurst(benchmark::State& state, secs::StorageMode mode)
{
    const int n = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        secs::Engine engine(mode);
        auto& proc = engine.GetEntityProcessor();
        for (int i = 0; i < n; ++i)
        {
            secs::Entity e = proc.AddEntity();
            benchmark::DoNotOptimize(proc.AddComponent<Pos>(e));
        }
        proc.ForceApplyChanges();
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * n);
    // Spin up one final engine to sample AllocatedBytes after N allocs.
    {
        secs::Engine sample(mode);
        auto& sproc = sample.GetEntityProcessor();
        for (int i = 0; i < n; ++i)
            sproc.AddComponent<Pos>(sproc.AddEntity());
        sproc.ForceApplyChanges();
        state.counters["mem_bytes"] = benchmark::Counter(
            static_cast<double>(sproc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
            benchmark::Counter::kDefaults,
            benchmark::Counter::OneK::kIs1024);
    }
}
BENCHMARK_CAPTURE(BM_AllocBurst, Vector,    secs::StorageMode::Vector)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_AllocBurst, SparseSet, secs::StorageMode::SparseSet)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);

// ---------------------------------------------------------------------------
// 4. FreeComponent burst
//    Adds N entities with Pos, then removes the component from each.
//    For SparseSet this exercises the swap-with-last path.
//    For Vector, FreeComponent is a no-op (slot stays alive), so the
//    asymmetry in results is itself informative.
// ---------------------------------------------------------------------------
static void BM_FreeBurst(benchmark::State& state, secs::StorageMode mode)
{
    const int n = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        secs::Engine engine(mode);
        auto entities = BuildEngine(engine, n);
        auto& proc = engine.GetEntityProcessor();

        // Remove Pos from every entity.
        for (const auto& e : entities)
            proc.RemoveComponent<Pos>(e);

        proc.ForceApplyChanges();
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * n);
    // After removal the storage may have shrunk (SparseSet) or not (Vector).
    // Build a representative final state to sample AllocatedBytes.
    {
        secs::Engine sample(mode);
        auto entities2 = BuildEngine(sample, n);
        auto& sproc = sample.GetEntityProcessor();
        for (const auto& e : entities2)
            sproc.RemoveComponent<Pos>(e);
        sproc.ForceApplyChanges();
        state.counters["mem_bytes"] = benchmark::Counter(
            static_cast<double>(sproc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
            benchmark::Counter::kDefaults,
            benchmark::Counter::OneK::kIs1024);
    }
}
BENCHMARK_CAPTURE(BM_FreeBurst, Vector,    secs::StorageMode::Vector)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_FreeBurst, SparseSet, secs::StorageMode::SparseSet)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);

// ---------------------------------------------------------------------------
// 5. Mixed workload (add / update / remove cycle)
//    Each outer benchmark iteration:
//      - Reads and writes Pos for 100% of live entities       (update)
//      - Adds Vel to the first 30% that lack it               (add)
//      - Removes Pos from the last 20%                        (remove)
//    Approximates a real game-loop tick.
// ---------------------------------------------------------------------------
static void BM_MixedWorkload(benchmark::State& state, secs::StorageMode mode)
{
    const int n = static_cast<int>(state.range(0));
    secs::Engine engine(mode);
    auto entities = BuildEngine(engine, n);
    auto& proc = engine.GetEntityProcessor();

    const int addCount    = n * 30 / 100;
    const int removeCount = n * 20 / 100;

    for (auto _ : state)
    {
        // Update
        for (const auto& e : entities)
        {
            auto& p = proc.GetComponent<Pos>(e);
            p.x += 1.f;
            benchmark::DoNotOptimize(p);
        }

        // Add Vel to first 30%
        for (int i = 0; i < addCount; ++i)
            benchmark::DoNotOptimize(proc.AddComponent<Vel>(entities[i]));

        // Remove Pos from last 20%
        for (int i = n - removeCount; i < n; ++i)
            proc.RemoveComponent<Pos>(entities[i]);

        proc.ForceApplyChanges();
    }
    state.SetItemsProcessed(state.iterations() * n);
    state.counters["mem_bytes"] = benchmark::Counter(
        static_cast<double>(proc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
        benchmark::Counter::kDefaults,
        benchmark::Counter::OneK::kIs1024);
}
BENCHMARK_CAPTURE(BM_MixedWorkload, Vector,    secs::StorageMode::Vector)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_MixedWorkload, SparseSet, secs::StorageMode::SparseSet)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);

// ---------------------------------------------------------------------------
// 6. Sparse EID access
//    Creates entities with widely-spaced EIDs (by adding and removing many
//    entities before the real set, forcing high recycled IDs).
//    Highlights Vector memory waste and TLB pressure vs SparseSet stability.
// ---------------------------------------------------------------------------
static void BM_SparseIDs(benchmark::State& state, secs::StorageMode mode)
{
    const int n    = static_cast<int>(state.range(0));
    const int gap  = 50;   // discard this many between each live entity

    secs::Engine engine(mode);
    auto& proc = engine.GetEntityProcessor();

    // Burn through EIDs to create a sparse distribution.
    for (int i = 0; i < n * gap; ++i)
    {
        secs::Entity e = proc.AddEntity();
        proc.RemoveEntity(e);
    }
    proc.ForceApplyChanges();

    // Now create the real surviving entities — they have high, sparse EIDs.
    std::vector<secs::Entity> entities;
    entities.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        secs::Entity e = proc.AddEntity();
        proc.AddComponent<Pos>(e).x = static_cast<float>(i);
        entities.push_back(e);
    }
    proc.ForceApplyChanges();

    for (auto _ : state)
    {
        float acc = 0.f;
        for (const auto& e : entities)
        {
            auto& p = proc.GetComponent<Pos>(e);
            benchmark::DoNotOptimize(p);
            acc += p.x;
        }
        benchmark::DoNotOptimize(acc);
    }
    state.SetItemsProcessed(state.iterations() * n);
    state.counters["mem_bytes"] = benchmark::Counter(
        static_cast<double>(proc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
        benchmark::Counter::kDefaults,
        benchmark::Counter::OneK::kIs1024);
}
/*
BENCHMARK_CAPTURE(BM_SparseIDs, Vector,    secs::StorageMode::Vector)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SparseIDs, SparseSet, secs::StorageMode::SparseSet)
    ->RangeMultiplier(10)->Range(100, 10'000)->Unit(benchmark::kNanosecond);
*/

// ---------------------------------------------------------------------------
// 7. Partial component coverage
//    N total entities exist in the world; Pos is assigned to every `stride`-th
//    entity so the Pos owners are spread across the full EID range [0, N-1].
//    All entities carry Vel so the engine holds live state at every EID.
//
//    Key asymmetry:
//      VectorComponentStorage<Pos> must size to max(Pos-EID) ≈ N-1  →  capacity ≈ N
//      SparseSetComponentStorage<Pos> only stores K = N/stride components →  capacity ≈ K
//
//    mem_bytes therefore diverges by the factor `stride`, becoming dramatic at
//    large N — exactly the common ECS scenario where many entity archetypes
//    share a subset of components.
// ---------------------------------------------------------------------------
static void BM_PartialCoverage(benchmark::State& state, secs::StorageMode mode, int stride)
{
    const int n = static_cast<int>(state.range(0));

    secs::Engine engine(mode);
    auto& proc = engine.GetEntityProcessor();

    std::vector<secs::Entity> withPosEntities;
    withPosEntities.reserve(n / stride + 1);

    for (int i = 0; i < n; ++i)
    {
        secs::Entity e = proc.AddEntity();
        proc.AddComponent<Vel>(e);      // every entity has Vel — fills the full EID range

        if (i % stride == 0)            // only every stride-th entity gets Pos
        {
            proc.AddComponent<Pos>(e).x = static_cast<float>(i);
            withPosEntities.push_back(e);
        }
    }
    proc.ForceApplyChanges();

    for (auto _ : state)
    {
        float acc = 0.f;
        for (const auto& e : withPosEntities)
        {
            auto& p = proc.GetComponent<Pos>(e);
            p.x += 1.f;
            benchmark::DoNotOptimize(p);
            acc += p.x;
        }
        benchmark::DoNotOptimize(acc);
    }

    const int withPos = static_cast<int>(withPosEntities.size());
    state.SetItemsProcessed(state.iterations() * withPos);
    state.counters["density_pct"] = 100 / stride;
    state.counters["mem_bytes"] = benchmark::Counter(
        static_cast<double>(proc.GetComponentStorageForComponentType<Pos>()->AllocatedBytes()),
        benchmark::Counter::kDefaults,
        benchmark::Counter::OneK::kIs1024);
}
// 10 % ownership: every 10th entity has Pos — capacity asymmetry ≈ 10×
BENCHMARK_CAPTURE(BM_PartialCoverage, Vector_10pct,    secs::StorageMode::Vector,    10)
    ->RangeMultiplier(10)->Range(1'000, 100'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_PartialCoverage, SparseSet_10pct, secs::StorageMode::SparseSet, 10)
    ->RangeMultiplier(10)->Range(1'000, 100'000)->Unit(benchmark::kNanosecond);
// 1 % ownership: every 100th entity has Pos — capacity asymmetry ≈ 100×
BENCHMARK_CAPTURE(BM_PartialCoverage, Vector_1pct,    secs::StorageMode::Vector,    100)
    ->RangeMultiplier(10)->Range(1'000, 100'000)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_PartialCoverage, SparseSet_1pct, secs::StorageMode::SparseSet, 100)
    ->RangeMultiplier(10)->Range(1'000, 100'000)->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();
