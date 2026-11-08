# secs — Code Audit

**Date:** 2026-11-08
**Scope:** `src/lib/secs/**`, `src/gtests`, `src/benchmarks`, build files, `doc/SPECS.md`
**Baseline commit:** `51ccc60`

---

## How to read this

Each finding is given:

- **Impact**: what goes wrong if it is left alone (crash/UB > silent wrong behaviour > performance > maintainability).
- **Effort**: **S** (under 1 hour), **M** (half a day), **L** (1 day or more).
- **Priority**: impact weighed against effort. High-impact, cheap fixes come first.

Findings are grouped into four tiers and sorted by priority within each tier.

| Tier | Meaning |
|------|---------|
| **P0** | Correctness bugs: undefined behaviour, data loss, or behaviour that contradicts the spec. Fix first. |
| **P1** | Performance: real costs on the per-frame hot path. |
| **P2** | API and design: footguns, dead code, ownership. |
| **P3** | Hygiene and tooling. |

---

## Summary table

| # | Tier | Finding | Impact | Effort |
|---|------|---------|--------|--------|
| 1 | P0 | `TagEntity` erases **all** tags whenever a new tag is introduced | High | S |
| 2 | P0 | Structural changes made inside system callbacks during `ApplyChanges` cause UB and are lost | High | M |
| 3 | P0 | Component references are invalidated by later `AddComponent` calls (both backends, including inside `Process`) | High | M |
| 4 | P0 | Calling `AddComponent` twice before `ApplyChanges` resets the data and queues duplicate edits | High | S |
| 5 | P0 | Components are never freed (`FreeComponent` is never called), so removed data and resources linger | High | M |
| 6 | P0 | No generation counter on entity IDs, so stale handles silently alias recycled entities | High | M |
| 7 | P0 | Systems added after entities exist never see those entities | Medium | S |
| 8 | P0 | `SetSystemGroup` asserts on the wrong thing and dereferences null | Medium | S |
| 9 | P0 | `GetComponent`/`RemoveComponent` on a missing component reads out of bounds; entity count drifts | Medium | S |
| 10 | P1 | Every `GetComponent` copies a `shared_ptr` (two atomic ops) and runs a resize check | High | S |
| 11 | P1 | `AcceptsEntity` heap-allocates (`vector<bool>`) per entity, per system, per change | Medium | M |
| 12 | P1 | System entity lists are `std::list` with O(n) removal | Medium | S |
| 13 | P1 | Iteration doesn't use the packed sparse-set arrays | Medium | L |
| 14 | P1 | Avoidable copies and O(n) operations in `EIDStorage`, `Visit`, `ApplyChanges` | Low–Med | S |
| 15 | P2 | Two conflicting `eid_t` types (`uint32_t` vs `uint64_t`) | Medium | S |
| 16 | P2 | Dead or broken code: `ai/` module, `ArrayComponentStorage`, `GetFlagForComponentType`, `Deactivate*` | Medium | S |
| 17 | P2 | `HasComponent` and `GetComponent` disagree before `ApplyChanges` | Medium | S |
| 18 | P2 | Ownership and lifetime: raw back-pointers, copyable `Engine`, no `RemoveSystem` | Medium | M |
| 19 | P2 | Global component-type registry (not thread-safe, breaks across DLLs, wastes bit 0) | Low–Med | M |
| 20 | P2 | Smaller API issues: no perfect forwarding, no emplace, `Pre/PostUpdate` on disabled systems, const-correctness | Low | S |
| 21 | P3 | Missing or unused includes, leftover debug code (`fflush(0)`), inconsistent include style | Low | S |
| 22 | P3 | Tests don't cover any of the bugs above; no CI, warnings or sanitizers | Medium | M |
| 23 | P3 | Docs are out of date (SPECS non-goals, freeing step, unused submodule) | Low | S |

**Suggested order:** 1 → 4 → 8 → 9 → 7 → 10 → 2 → 5 → 3 → 6, then P1 and P2 as time allows. Write each fix together with a regression test (#22).

---

## P0 — Correctness

### 1. `TagEntity` wipes every existing tag

[entityprocessor.h:216-223](../src/lib/secs/entityprocessor.h#L216-L223)

```cpp
if (m_taggedEntities.count(tag) == 0)
{
    m_taggedEntities = {};   // <-- clears the whole map, not just this tag
}
m_taggedEntities[tag].push_back(e);
```

**Failure:** `TagEntity(a, "player"); TagEntity(b, "enemy");` makes `GetAllEntitiesWithTag("player")` return empty.
The current test tags only one entity, so it doesn't catch this.

**Also:**
- Tags are never removed when an entity is destroyed. Combined with ID recycling (#6), a new entity inherits the old one's tag.
- Tagging the same entity twice adds a duplicate.
- `GetAllEntitiesWithTag` returns the vector by value.

**Fix:** delete the `if` block (`operator[]` already default-constructs). On entity removal, drop the entity from its tags (keep a reverse map `eid → tags`, or store tags as a component). Return `const std::vector<Entity>&`.

---

### 2. Structural changes inside callbacks during `ApplyChanges` cause UB and are lost

[entityprocessor.cpp:101-114](../src/lib/secs/entityprocessor.cpp#L101-L114)

`OnEntitiesAdded(m_addedEntities)` passes a reference to the processor's own member vector. Any user hook (`OnEntityAdded` / `OnEntityRemoved`) that calls `AddEntity`, `AddComponent`, `RemoveComponent` or `RemoveEntity`:

1. `push_back`s into the vector currently being iterated. This is **undefined behaviour** (iterator invalidation).
2. Is then thrown away by the `clear()` calls at the end of `ApplyChanges`. The new entity is never announced to systems, and its component flags are never set, so it never matches any system.

These hooks are the natural place for such calls (spawning a child entity, adding a "just spawned" marker component, tearing down a physics body), so this will happen in real use.

**Fix:** swap the pending buffers into locals before notifying, and loop until nothing new is queued:

```cpp
void EntityProcessor::ApplyChanges()
{
    while (!m_addedEntities.empty() || !m_removedEntities.empty() || !m_componentEdits.empty())
    {
        auto added   = std::exchange(m_addedEntities,   {});
        auto removed = std::exchange(m_removedEntities, {});
        auto edits   = std::exchange(m_componentEdits,  {});
        // ... same processing, using the locals ...
    }
}
```

Add an iteration cap with an assert to catch infinite spawn loops.

---

### 3. Component references are invalidated by later additions

[componentstorage.h:140-156](../src/lib/secs/componentstorage.h#L140-L156) (SparseSet `push_back`), [componentstorage.h:222-245](../src/lib/secs/componentstorage.h#L222-L245) (Vector `resize`), [entitysystem.h:169-172](../src/lib/secs/entitysystem.h#L169-L172)

Both backends store components in a `std::vector` that can reallocate. Every `T&` handed out earlier then dangles:

```cpp
auto& p1 = proc.AddComponent<Position>(e1);
auto& p2 = proc.AddComponent<Position>(e2);   // may reallocate
p1.x = 5;                                       // use-after-free
```

The more dangerous case is inside a system:

```cpp
void Process(double, const Entity& e, Position& pos, Velocity& vel) override {
    auto bullet = GetEntityProcessor()->AddEntity();
    AddComponent<Position>(bullet) = pos;   // reallocates the Position pool
    pos.x += vel.vx;                          // pos now dangles
}
```

The SparseSet backend is the **default**, and it also moves elements on removal (swap-with-last), so pointers held across frames are never safe either.

**Fix options, cheapest first:**
1. **Document it** in `AddComponent`/`Process`: "references are valid until the next structural change to that component type".
2. **Paged storage:** allocate components in fixed-size pages (e.g. 1024 per page) that never move. Element addresses stay stable on growth. Removal still needs either tombstones or the documented swap caveat.
3. **Defer allocation:** combined with #2, queue component construction during `Step` so pools only grow inside `ApplyChanges`. Then references are stable for the whole frame. This matches the spirit of SPECS §5.3.

Option 2 is the usual approach (EnTT uses paged storage for this reason).

---

### 4. Calling `AddComponent` twice before `ApplyChanges` resets the data

[entityprocessor.h:107-117](../src/lib/secs/entityprocessor.h#L107-L117)

`AddOrRetrieveComponent` checks `HasComponent`, which reads the *flags*. Flags are only set in `ApplyChanges`. Within one frame, a second call sees "not present", calls `AllocComponent` again (which resets the value to `{}`), and queues a duplicate edit.

```cpp
proc.AddComponent<Health>(e).hp = 50;
proc.AddOrRetrieveComponent<Health>(e);   // hp is now 100 again
```

The same mismatch makes `RemoveComponent` a no-op for a component added in the same frame (the flag isn't set yet, so no remove edit is queued). The entity ends up *with* the component.

**Fix:** set the flag immediately in `AddComponent`/`RemoveComponent` (SPECS §5.3 already says "there might be no reason for not updating entity bits here"), and keep only the **system notification** deferred. Alternatively, ask the storage `Contains(e)`. Either way, assert in `AddComponent` that the component isn't already present (the assert is currently commented out at line 96).

---

### 5. Components are never freed

[componentstorage.h:171-189](../src/lib/secs/componentstorage.h#L171-L189), [entityprocessor.cpp:105-109](../src/lib/secs/entityprocessor.cpp#L105-L109)

`FreeComponent` exists but nothing in the library calls it. `RemoveComponent` and `RemoveEntity` only clear flag bits. As a result:

- Destructors never run. A component holding a `shared_ptr<Texture>`, a file handle or a physics body keeps that resource alive until the entity ID is reused, or forever.
- The SparseSet dense array fills up with dead entries, which defeats its purpose (it is supposed to be packed) and would break any dense iteration (#13).
- This contradicts SPECS §5 ("…and finally, components will be freed").

**Fix:** in `ApplyChanges`, *after* notifying systems (so `OnEntityRemoved` can still read the data, as the spec intends):
- for each `RemoveComponent` edit, call `pool->FreeComponent(e)`;
- for each removed entity, free every component whose bit is set.

This needs a type-erased `virtual void Free(const Entity&)` on `BaseComponentStorage` so the processor can free by type index. `VectorComponentStorage::FreeComponent` should at least reset the slot to `T{}` so resources are released.

---

### 6. No generation counter on entity IDs

[eidstorage.cpp:7-28](../src/lib/secs/eidstorage.cpp#L7-L28), [entity.h](../src/lib/secs/entity.h)

IDs are recycled LIFO. An `Entity` held by game code (a target, parent or owner reference) silently starts pointing at whatever entity gets that ID next. There's no way to ask "is this handle still alive?".

**Fix:** split `Entity` into `index:32 | generation:32` (the 64-bit type is already there, see #15). `EIDStorage` keeps `std::vector<uint32_t> generations`, bumping the generation on recycle. Add `EntityProcessor::IsAlive(Entity)`. Storages index by `e.Index()` only. This is the standard ECS approach (EnTT, flecs, Bevy).

---

### 7. Systems added after entities exist never see them

[engine.h:67-78](../src/lib/secs/engine.h#L67-L78)

`PushSystem` only registers the system for *future* notifications. Entities created before the system was added are never offered to it, even if they match. This is easy to hit with level loading, or systems created lazily by game state.

**Fix:** at the end of `PushSystem`, call `entity_system->OnEntitiesAdded(currentValidEntities)`.

---

### 8. `SetSystemGroup` asserts on the wrong thing

[systemmanager.cpp:74-79](../src/lib/secs/systemmanager.cpp#L74-L79)

```cpp
assert( m_systemsByGroup != nullptr && ... );   // array decays to pointer: always true
assert( group < MaxSystemGroups );               // checked after the bad index is already possible
m_systemsByGroup[group]->push_back(system);      // null deref if group not activated
```

`DisableSystemGroup`/`EnableSystemGroup` don't bounds-check `sgi` either.

**Fix:** replace `std::vector<System::Ptr>* m_systemsByGroup[32]` + `new`/`delete` with `std::array<std::vector<System::Ptr>, MaxSystemGroups>` and an `std::bitset<MaxSystemGroups> m_activeGroups`. That removes the manual destructor, removes the null case, and lets the asserts check the right thing.

---

### 9. Out-of-bounds reads and entity count drift

- **Missing component:** `SparseSetComponentStorage::GetComponent` ([componentstorage.h:162-165](../src/lib/secs/componentstorage.h#L162-L165)) indexes `m_sparse[eid]` with no bounds check and doesn't check for `InvalidIndex`. `RemoveComponent` ([entityprocessor.h:126-134](../src/lib/secs/entityprocessor.h#L126-L134)) always returns `GetComponent(e)`, even when the entity never had the component. That is an out-of-bounds read.
  **Fix:** add `assert(Contains(e))` to `GetComponent`. Make `RemoveComponent` return `void` (or `bool`), and add a `T* TryGetComponent(e)`.
- **Unknown entity:** `ComponentFlagsManager::GetComponentFlagsForEntity` ([componentflagsmanager.cpp:24-27](../src/lib/secs/componentflagsmanager.cpp#L24-L27)) is unchecked, so `HasComponent` on an out-of-range entity is UB.
- **Entity count:** `RemoveEntity` ([entityprocessor.cpp:36-40](../src/lib/secs/entityprocessor.cpp#L36-L40)) decrements `m_numEntities` immediately and on every call. Removing the same entity twice drives the count below the real value (the removal list is later deduplicated, but the counter isn't). The count also disagrees with `ValidEntities()`, which only updates in `ApplyChanges`.
  **Fix:** derive the count from `EIDStorage` and drop the separate counter.

---

## P1 — Performance

SPECS §4 lists "Fast" as a non-goal, but the recent SparseSet work and benchmarks show that performance now matters. These are the main hot-path costs.

### 10. `GetComponent` copies a `shared_ptr` on every call

[componentmanager.h:36-76](../src/lib/secs/componentmanager.h#L36-L76), [entitysystem.h:98-102](../src/lib/secs/entitysystem.h#L98-L102)

Every `GetComponent<T>(e)`, including the one `TypedEntitySystem::Process` makes for each component of each entity every frame, runs:
1. a size check, with a possible `resize` of `m_componentStorageVector`;
2. a null check;
3. `std::static_pointer_cast`, which constructs a new `shared_ptr` (**atomic increment**) and destroys it at the end of the expression (**atomic decrement**).

**Fix:**
- Return a raw `TypedComponentStorage<T>&` from `ComponentManager` (it owns the pools; nobody else needs shared ownership). Store pools as `std::unique_ptr<BaseComponentStorage>`.
- In `TypedEntitySystem`, cache `TypedComponentStorage<Args>*...` in a `std::tuple` once (on `PushSystem`), and fetch from the cached pointers in `Process`.
- Mark `GetComponent` on the concrete storages `final` so the compiler can devirtualize the call through the cached pointer, or drop the virtual interface entirely by making the backend a template parameter / compile-time choice.

Expect a large speedup in `BM_Iteration`, and in any system that touches more than one component.

### 11. `AcceptsEntity` allocates on every call

[entitysystem.cpp:46-50](../src/lib/secs/entitysystem.cpp#L46-L50), [dynamicbitset.h](../src/lib/secs/dynamicbitset.h)

`(m_neededComponents & bits) == m_neededComponents` builds a temporary `DynamicBitset` (a heap-allocated `std::vector<bool>`) and then compares it bit by bit through proxy references. This runs for every changed entity × every system each frame. `operator|` used in `ComponentBitsBuilder` has the same issue.

**Fix:** back `DynamicBitset` with `std::vector<uint64_t>` (or a small-buffer inline `uint64_t[2]` for ≤128 component types) and add an allocation-free `bool IsSubsetOf(const DynamicBitset&) const` that compares word by word. `Set`, `Test`, `|=` and `==` all become word operations.

### 12. System entity lists are `std::list`

[entitysystem.h:130](../src/lib/secs/entitysystem.h#L130), [entitysystem.cpp:99-105](../src/lib/secs/entitysystem.cpp#L99-L105)

`std::list<Entity>` puts each 8-byte entity in its own heap node, which is slow to iterate. `RemoveEntity` uses `std::remove` over the whole list, so removal is O(n).

**Fix:** use a sparse set of entities (`std::vector<Entity> dense` + `std::vector<uint32_t> sparse`) with swap-and-pop removal, which is O(1). It also makes `m_entityBits` unnecessary, because `sparse` answers "is present". If deterministic iteration order matters, keep a vector and use tombstones, then compact at the end of `ApplyChanges`.

### 13. Iteration doesn't use the packed arrays

The SparseSet backend keeps components tightly packed, but systems still iterate *their entity list* and do a random `sparse → dense` lookup for every component. The cache benefit of packing is lost.

**Fix:** add a view/query API that iterates the *smallest* pool among the requested types, walks its `m_denseEntities` in order, and checks the others with `Contains`:

```cpp
engine.Each<Position, Velocity>([](Entity e, Position& p, Velocity& v) { ... });
```

`TypedEntitySystem` can then be built on top of it. This depends on #5 (no dead entries in dense arrays) and #10 (no `shared_ptr` overhead).

### 14. Smaller hot-path costs

- `EIDStorage::validEntities()` ([eidstorage.h:54-57](../src/lib/secs/eidstorage.h#L54-L57)), `EntityProcessor::ValidEntities()` and `Engine::Visit` each **copy** the whole vector. Return `const std::vector<eid_t>&`.
- `EIDStorage::Recycle` uses `erase(remove(...))`, which is O(n) per removed entity. Use a swap-and-pop sparse set, or an `alive` bitset plus lazy compaction.
- `ApplyChanges` builds a `std::list`, sorts it, copies it into a vector, and sorts again. Collect into a `std::vector`, `sort`, then `unique` once. Better still, mark changed entities in a bitset/vector to avoid sorting.
- `Engine::Visit` takes a `std::function` by value. Make it a template parameter `F&&`.
- `std::stack` (on `std::deque`) for recycled IDs. A `std::vector` used as a stack is cheaper.

---

## P2 — API and design

### 15. Two conflicting `eid_t` types

[entity.h:9](../src/lib/secs/entity.h#L9) defines `secs::eid_t = uint32_t`. [entity.h:16](../src/lib/secs/entity.h#L16) defines `secs::Entity::eid_t = uint64_t`. `GetEID()` returns the 64-bit type. The code base is full of `secs::eid_t(entity.GetEID())` casts that silently truncate. `EIDStorage` hands out 32-bit IDs that are widened into 64-bit entities.

**Fix:** keep one type. If #6 is implemented, `Entity` holds a `uint64_t` with `Index()` (32-bit) and `Generation()` (32-bit) accessors, and every storage uses `Index()`.

### 16. Dead or broken code

| Item | Location | Problem | Action |
|------|----------|---------|--------|
| `ai/ComponentBehaviour` | [ai/componentbehaviour.h:25-33](../src/lib/secs/ai/componentbehaviour.h#L25-L33) | `AddOrRetrieveComponent<ComponentTypes...>` fails to compile with more than one type; `save(...)` is called with the wrong arguments; compiles today only because it is never instantiated. Circular include with `secs.h`. `IComponentBehaviour` has no virtual destructor. | Remove from `secs.h`, then fix or delete |
| `ArrayComponentStorage` | [componentstorage.h:34-76](../src/lib/secs/componentstorage.h#L34-L76) | Unused; fixed 1024 entities; doc comment says it resizes (it doesn't) | Delete |
| `GetFlagForComponentType` | [componenttraits.h:22-26](../src/lib/secs/componenttraits.h#L22-L26) | Unused; uses floating-point `pow` without `<cmath>`; overflows `uint32_t` past 31 types | Delete |
| `Engine::Deactivate`, `EntityProcessor::DeactivateEntity` | [engine.h:172-175](../src/lib/secs/engine.h#L172-L175), [entityprocessor.h:152-155](../src/lib/secs/entityprocessor.h#L152-L155) | No-ops: entities are collected and then cleared without effect | Implement (e.g. a "disabled" tag that makes systems skip the entity) or delete |
| `TypedEntitySystem::m_componentManager` | [entitysystem.h:183](../src/lib/secs/entitysystem.h#L183) | Uninitialized private member that shadows the base class one | Delete |
| `SECS_VECTOR_STORAGE_LEGACY_GROWTH` | [componentstorage.h:224-244](../src/lib/secs/componentstorage.h#L224-L244) | `#define`d inside the function, so the `#else` branch is dead; comment says "+32", code does "+8" | Pick one path. `std::vector::resize` already grows geometrically, so plain `resize(eid + 1)` is enough |
| `SECS_COMPONENT` macro, `config.h` | [system.h:7](../src/lib/secs/system.h#L7) | Unused | Delete |

### 17. `HasComponent` and `GetComponent` disagree within a frame

After `AddComponent<T>(e)`, `GetComponent<T>(e)` works immediately but `HasComponent<T>(e)` returns `false` until `ApplyChanges`. After `RemoveComponent`, the reverse happens. This causes #4 and is surprising to users.

**Fix:** apply flags immediately and defer only the system notifications (see #4). If deferred flags are kept on purpose, document it on both methods.

### 18. Ownership and lifetime

- `EntitySystem` holds raw pointers to `Engine` members ([entitysystem.h:131-133](../src/lib/secs/entitysystem.h#L131-L133)). `CreateSystem` returns a `shared_ptr` that can outlive the `Engine`, leaving those pointers dangling.
- `Engine` is implicitly copy-constructible. A copy's `EntityProcessor`/`SystemManager` hold references into the **original** engine's managers. Add `Engine(const Engine&) = delete; Engine& operator=(const Engine&) = delete;` (same for move, or implement them properly).
- No `RemoveSystem`. Systems can only be disabled.
- `Engine::PushSystem` uses `dynamic_pointer_cast` to discover `EntitySystem`. That's fine, but `CreateSystem<T>` already knows the static type and could use `if constexpr (std::is_base_of_v<EntitySystem, T>)`.

### 19. Global component-type registry

[componenttraits.h:33-37](../src/lib/secs/componenttraits.h#L33-L37), [componenttraits.cpp:7](../src/lib/secs/componenttraits.cpp#L7)

- `nextTypeIndex++` is not atomic. Two threads registering a type for the first time race.
- Function-local statics in header templates get **one copy per DLL** on Windows. A component type used from both a DLL and the exe can get two different indices.
- Indices start at **1**, so bit 0 of every bitset is wasted (declared `ctindex_t`, defined as `ctflags_t`. Same type today, but inconsistent).
- Indices are global, not per-`Engine`, so two engines share the numbering. That's harmless, but it means bitsets size to the global count.

**Fix:** start at 0 and use `std::atomic<ctindex_t>`. For the DLL issue, either document "single-module only", or key types by a stable compile-time hash (e.g. of `__PRETTY_FUNCTION__`/`__FUNCSIG__`) mapped to per-engine indices.

### 20. Smaller API issues

- `CreateSystem` forwards `args...` instead of `std::forward<Args>(args)...` ([engine.h:58](../src/lib/secs/engine.h#L58)).
- No `AddComponent<T>(e, args...)` / emplace. Components must be default-constructible and are always value-initialized, then overwritten.
- `SystemManager::Step` calls `PreUpdate`/`PostUpdate` on **disabled** systems ([systemmanager.cpp:102-110](../src/lib/secs/systemmanager.cpp#L102-L110)). Probably unintended.
- `SetExecutionPriority` only has an effect if called *before* `PushSystem` (insertion is sorted once). Document this, or re-sort on change.
- Const-correctness: `ComponentEdit::GetType/GetIndex/GetEntity`, `System::IsEnabled/HasProcessingStep/GetExecutionPriority`, `ComponentFlagsManager::HasComponent` should be `const`.
- `EntityObserver` interface plus `final` overrides on `EntitySystem` mean users override `OnEntityAdded`, while `OnEntitiesAdded` is the observer API. Naming is confusing; consider `OnEntityEnter`/`OnEntityExit`.
- `GetNumEntities` returns `int`; IDs are unsigned. Use `size_t`.

---

## P3 — Hygiene and tooling

### 21. Includes and leftovers

- Missing includes, compiling today only through transitive includes: `<string>` in [entityprocessor.h](../src/lib/secs/entityprocessor.h), `<cassert>` in [systemmanager.cpp](../src/lib/secs/systemmanager.cpp) and [entityprocessor.h](../src/lib/secs/entityprocessor.h), `<limits>` in [componentstorage.h](../src/lib/secs/componentstorage.h), `<cstdint>` in [componenttraits.h](../src/lib/secs/componenttraits.h), `<cstdio>` for `fflush`.
- Unused `<iostream>` in [componentmanager.h](../src/lib/secs/componentmanager.h), [entitysystem.h](../src/lib/secs/entitysystem.h), [componentflagsmanager.cpp](../src/lib/secs/componentflagsmanager.cpp). It adds compile time and static init.
- `fflush(0);` debug leftover in [entitysystem.cpp:104](../src/lib/secs/entitysystem.cpp#L104). Also several commented-out `std::cout` lines.
- [entityprocessor.cpp:1](../src/lib/secs/entityprocessor.cpp#L1) includes `"secs/entityprocessor.h"`; every other file uses relative includes. Pick one style.
- Empty translation units: `componentmanager.cpp`, `componentstorage.cpp`, `ai/componentbehaviour.cpp`.
- Mixed tabs and spaces throughout. Add a `.clang-format`.

### 22. Tests, CI and warnings

The test suite covers the happy paths well, but none of the P0 issues. Add a regression test with each fix:

- tagging two different tags (#1);
- spawning an entity / adding a component from `OnEntityAdded` (#2);
- adding to a pool while holding a reference, under ASan (#3);
- calling `AddComponent` twice in a frame, and `Add` then `Remove` in the same frame (#4);
- destructor called on remove: a component with a `shared_ptr` member, checking `use_count` (#5);
- stale handle after recycle (#6);
- system created after entities exist (#7).

Note that `EntityCountDecreasesOnRemove` currently locks in the immediate-decrement behaviour from #9.

Build and CI:
- Enable warnings: `/W4` (MSVC) or `-Wall -Wextra -Wpedantic` (GCC/Clang), plus `-Werror` in CI. Several issues above (shadowed member, unused params) would show up as warnings.
- Add an ASan + UBSan configuration. #2, #3 and #9 are exactly what sanitizers catch.
- Add a GitHub Actions workflow (Windows + Linux) that runs tests and a short benchmark smoke test.
- `file(GLOB_RECURSE …)` doesn't pick up new files without re-running CMake. List sources explicitly, or add `CONFIGURE_DEPENDS`.
- Add `target_compile_features(secs PUBLIC cxx_std_17)` so consumers inherit the standard requirement.

### 23. Documentation

- SPECS §4 lists "Fast" as a non-goal. The recent SparseSet backend and benchmarks contradict that. Update the stated goals.
- SPECS §5 describes freeing components after notification, which isn't implemented (#5).
- `module/xstf` submodule and `cmake/TestHelpers.cmake` are unused (all references are commented out). Remove them, or document why they stay. The submodule uses an SSH URL, which breaks anonymous clones.
- README has no build instructions or usage example. A 20-line example (create engine, add entity and components, write a `TypedEntitySystem`, step) would help a lot.

---

## Appendix: suggested target architecture

If the P0 and P1 items are done together, the core ends up looking like this:

```
Entity            = { uint32 index; uint32 generation; }
EntityRegistry    : generations[], free list, alive sparse set
Pool<T>           : paged sparse set (stable addresses), Contains/Get/TryGet/Emplace/Remove,
                    type-erased base with virtual Remove(Entity) for bulk entity destruction
ComponentMask     : word-based bitset, IsSubsetOf()
EntityProcessor   : immediate flag updates; deferred *notifications* via double-buffered queues,
                    drained in a loop; frees components after notifying
EntitySystem      : sparse-set membership (O(1) add/remove), cached Pool<T>* per required type
View<Ts...>       : iterate smallest pool, filter by others
```

This keeps the current public shape (`Engine`, `EntityProcessor`, `TypedEntitySystem`, deferred structural changes) while removing the UB and most of the per-frame overhead.
