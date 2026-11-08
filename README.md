# secs

Aiming to be a simple ECS framework for games, and somehow based on others like **[artemis-odb](https://github.com/junkdog/artemis-odb)**, **[arethusa](https://github.com/junkdog/arethusa-ecs)** or **[ashley](https://github.com/libgdx/ashley)**. Made for fun and non-profit.

- C++17, no dependencies for the library itself.
- Systems match entities by component set (`TypedEntitySystem<Position, Velocity>`).
- Structural changes (create and remove entities, add and remove components) are visible immediately to queries such as `HasComponent` and `GetComponent`. Systems are notified once per frame, at the start of `Engine::Step`.
- Two component storage backends: a packed sparse set (default) and storage indexed directly by entity index. In both, component addresses stay stable while other components are added.
- Generational entity handles: a handle to a removed entity never aliases the entity that reuses its slot (`EntityProcessor::IsAlive`).

See the [design specs](doc/SPECS.md) for the life cycle and the reasoning behind it, and the [code audit](doc/AUDIT.md) for known issues and their status.

## Usage

```cpp
#include <secs/secs.h>
#include <cstdio>

struct Position { float x = 0.f, y = 0.f; };
struct Velocity { float vx = 0.f, vy = 0.f; };

// Processes every entity that has both a Position and a Velocity.
class MovementSystem : public secs::TypedEntitySystem<Position, Velocity>
{
public:
    SECS_SYSTEM(MovementSystem)

    void Process(double delta, const secs::Entity&, Position& pos, Velocity& vel) override
    {
        pos.x += vel.vx * static_cast<float>(delta);
        pos.y += vel.vy * static_cast<float>(delta);
    }
};

int main()
{
    secs::Engine engine;                       // or secs::Engine(secs::StorageMode::Vector)
    engine.CreateSystem<MovementSystem>();

    auto& proc = engine.GetEntityProcessor();
    secs::Entity e = proc.AddEntity();
    proc.AddComponent<Position>(e);
    proc.AddComponent<Velocity>(e).vx = 60.f;

    for (int frame = 0; frame < 3; ++frame)
    {
        engine.Step(1.0 / 60.0);               // apply pending changes, then run systems
    }
    std::printf("x = %.1f\n", proc.GetComponent<Position>(e).x);   // x = 3.0

    proc.RemoveEntity(e);
    engine.Step(1.0 / 60.0);
    std::printf("alive = %d\n", proc.IsAlive(e));                   // alive = 0
}
```

Other things worth knowing:

- `EntitySystem` (the untyped base) lets you override `AcceptsEntity`, `OnEntityAdded`, `OnEntityRemoved`, `Process` and `Render`. Structural changes made from these callbacks are processed in the same `Step`.
- `TryGetComponent<T>(e)` returns `nullptr` when the entity doesn't have the component. `RemoveComponent<T>(e)` returns whether there was one to remove.
- Removed components stay readable until systems have been notified (so `OnEntityRemoved` can clean up), and are freed right after.
- References returned by `AddComponent` and `GetComponent` stay valid while other components are added. Removing a component may move another one of the same type, so don't keep references across `Step`.
- Systems can be grouped (`Engine::SetSystemGroup`) and enabled or disabled per group, and ordered with `SetExecutionPriority`.

## Building

Requirements: CMake 3.15+, a C++17 compiler, and [Conan 2](https://conan.io/) for the test and benchmark dependencies (GoogleTest, Google Benchmark).

```bash
scripts/build-all.sh [Debug|Release]          # library + tests + benchmarks
scripts/run-audit-tests.sh                    # audit regression tests (see doc/AUDIT.md)
./run.sh                                      # build and run the full test suite
./run-bench.sh Release                        # build and run the benchmarks
./build.sh                                    # library only
```

On Windows, use the PowerShell equivalents: `scripts\build-all.ps1 -BuildType Release` and `scripts\run-audit-tests.ps1`.

The full test suite is the `secs-tests` binary in the build folder (`build/Debug/secs-tests` with multi-config generators such as Visual Studio). Run it with `--gtest_filter=...` to select tests. Run the tests in Debug: some audit tests rely on Debug bounds checks.

To use the library from another CMake project, add this repository as a subdirectory and link the `secs` target:

```cmake
add_subdirectory(secs)
target_link_libraries(my_game PRIVATE secs)
```
