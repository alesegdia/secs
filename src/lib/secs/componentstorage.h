#pragma once

#include <vector>
#include <cassert>
#include <memory>

#include "entity.h"
#include "bits.h"

namespace secs {

    /**
     * @class BaseComponentStorage
     * @brief Base class for component storage.
     */
    class BaseComponentStorage {
    public:

        typedef std::unique_ptr<BaseComponentStorage> UniquePtr;
        typedef BaseComponentStorage* Ptr;
        typedef std::shared_ptr<BaseComponentStorage> SharedPtr;

        virtual ~BaseComponentStorage() {}

    };

    /**
     * @brief Component addition and deletion processes are split into two steps.
     * 1) Set/clear the bit associated with the entity.
     * 2) Allocate/free the component associated with the entity.
     *
     * @tparam ComponentType The type of the component.
     */
    template <typename ComponentType>
    class ArrayComponentStorage : public BaseComponentStorage
    {
    public:

        using UniquePtr = std::unique_ptr<ArrayComponentStorage<ComponentType>>;
        using SharedPtr = std::shared_ptr<ArrayComponentStorage>;
        using Ptr = ArrayComponentStorage<ComponentType>*;

        static constexpr size_t NumEntities = 1024;

        /**
         * @brief Allocates and returns a component for the entity passed as a parameter.
         * Resizes the container if the entity does not fit.
         * @param e The entity for which the component is to be allocated.
         * @return The allocated component.
         */
        ComponentType& AllocComponentForEntity(const Entity& e)
        {
            assert(e.GetEID() < ArrayComponentStorage::NumEntities);
            m_storage[e.GetEID()] = {};
            return m_storage[e.GetEID()];
        }

        /**
         * @brief Retrieves the component for a given entity.
         * @param e The entity for which the component is to be retrieved.
         * @return A reference to the component of the specified type for the given entity.
         */
        ComponentType& GetComponentForEntity(const Entity& e)
        {
            return m_storage[e.GetEID()];
        }

    private:

        /**
         * @brief Storage for components. The container is resized when a component is
         * requested for an entity and it does not fit.
         */
        ComponentType m_storage[ArrayComponentStorage::NumEntities];

    };

    /**
     * @brief Selects which concrete storage backend is created by ComponentManager.
     */
    enum class StorageMode
    {
        Vector,    ///< VectorComponentStorage — flat vector indexed by eid (legacy behaviour)
        SparseSet  ///< SparseSetComponentStorage — packed dense array + sparse index (default)
    };

    /**
     * @brief Pure-virtual typed base that unifies VectorComponentStorage and
     * SparseSetComponentStorage behind a single interface used by ComponentManager.
     *
     * @tparam ComponentType The type of the component.
     */
    template <typename ComponentType>
    class TypedComponentStorage : public BaseComponentStorage
    {
    public:
        using SharedPtr = std::shared_ptr<TypedComponentStorage<ComponentType>>;
        using Ptr       = TypedComponentStorage<ComponentType>*;

        virtual ComponentType& AllocComponent(const Entity& e) = 0;
        virtual ComponentType& GetComponent(const Entity& e)   = 0;
        virtual void           FreeComponent(const Entity& e)  = 0;

        /**
         * @brief Returns the number of bytes occupied by ComponentType elements
         *        currently allocated in memory (capacity, not just live slots).
         *        Use this to compare memory footprint between backends.
         */
        virtual size_t AllocatedBytes() const = 0;
    };

    /**
     * @brief Sparse-set component storage.
     *
     * Layout:
     *   m_sparse[eid]  → index into m_dense   (one array lookup)
     *   m_dense[index] → the actual ComponentType value (contiguous, no gaps)
     *
     * Properties:
     *   - O(1) random access via entity ID (1 indirection through the sparse array)
     *   - Components live in a tightly-packed contiguous array — cache-friendly iteration
     *   - FreeComponent swaps the removed slot with the last dense element so the
     *     dense array stays packed with no holes.
     *
     * @tparam ComponentType The type of the component.
     */
    template <typename ComponentType>
    class SparseSetComponentStorage : public TypedComponentStorage<ComponentType>
    {
    public:
        using UniquePtr = std::unique_ptr<SparseSetComponentStorage<ComponentType>>;
        using SharedPtr = std::shared_ptr<SparseSetComponentStorage<ComponentType>>;
        using Ptr       = SparseSetComponentStorage<ComponentType>*;

        static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

        /**
         * @brief Allocates (or resets) a component slot for the given entity.
         */
        ComponentType& AllocComponent(const Entity& e) override
        {
            const size_t eid = static_cast<size_t>(e.GetEID());
            GrowSparse(eid);

            if (m_sparse[eid] == InvalidIndex)
            {
                m_sparse[eid] = m_dense.size();
                m_dense.push_back({});
                m_denseEntities.push_back(e);
            }
            else
            {
                m_dense[m_sparse[eid]] = {};
            }
            return m_dense[m_sparse[eid]];
        }

        /**
         * @brief Retrieves the component for a given entity.
         *        One indirection: sparse[eid] → dense[index].
         */
        ComponentType& GetComponent(const Entity& e) override
        {
            return m_dense[m_sparse[static_cast<size_t>(e.GetEID())]];
        }

        /**
         * @brief Frees the component slot for the given entity.
         *        Uses swap-with-last to keep the dense array packed.
         */
        void FreeComponent(const Entity& e) override
        {
            const size_t eid      = static_cast<size_t>(e.GetEID());
            assert(eid < m_sparse.size() && m_sparse[eid] != InvalidIndex);

            const size_t denseIdx = m_sparse[eid];
            const size_t lastIdx  = m_dense.size() - 1;

            if (denseIdx != lastIdx)
            {
                m_dense[denseIdx]          = std::move(m_dense[lastIdx]);
                m_denseEntities[denseIdx]  = m_denseEntities[lastIdx];
                m_sparse[static_cast<size_t>(m_denseEntities[denseIdx].GetEID())] = denseIdx;
            }

            m_dense.pop_back();
            m_denseEntities.pop_back();
            m_sparse[eid] = InvalidIndex;
        }

        /**
         * @brief Returns bytes used by the packed dense component array (capacity).
         *        Does not include the sparse index or entity list overhead.
         */
        size_t AllocatedBytes() const override
        {
            return m_dense.capacity() * sizeof(ComponentType);
        }

    private:
        void GrowSparse(size_t eid)
        {
            if (eid >= m_sparse.size())
                m_sparse.resize(eid + 8, InvalidIndex);
        }

        std::vector<size_t>        m_sparse;        ///< sparse[eid] → index into m_dense
        std::vector<ComponentType> m_dense;          ///< packed component values
        std::vector<Entity>        m_denseEntities;  ///< entity at each dense slot (for swap-on-remove)
    };

    // Make VectorComponentStorage implement TypedComponentStorage so ComponentManager
    // can hold both backends behind the same pointer type.
    template <typename ComponentType>
    class VectorComponentStorage : public TypedComponentStorage<ComponentType>
    {
    public:
        using UniquePtr = std::unique_ptr<VectorComponentStorage<ComponentType>>;
        using SharedPtr = std::shared_ptr<VectorComponentStorage<ComponentType>>;
        using Ptr       = VectorComponentStorage<ComponentType>*;

        ComponentType& AllocComponent(const Entity& e) override
        {
#define SECS_VECTOR_STORAGE_LEGACY_GROWTH
#ifdef SECS_VECTOR_STORAGE_LEGACY_GROWTH
            // Legacy: linear growth by +32 slots each time
            if (m_storage.size() < (e.GetEID() + 1))
                m_storage.resize(secs::eid_t(e.GetEID() + 8));
            m_storage[secs::eid_t(e.GetEID())] = {};
            return m_storage[secs::eid_t(e.GetEID())];
#else
            // Geometric doubling starting at 64 — matches std::vector push_back semantics
            // and provides a fair baseline for memory comparisons with SparseSetComponentStorage.
            const auto required = static_cast<size_t>(e.GetEID()) + 1;
            if (required > m_storage.size())
            {
                size_t newCap = m_storage.capacity() < 64 ? 64ULL : m_storage.capacity() * 2;
                while (newCap < required) newCap *= 2;
                m_storage.reserve(newCap);
                m_storage.resize(required);
            }
            m_storage[static_cast<size_t>(e.GetEID())] = {};
            return m_storage[static_cast<size_t>(e.GetEID())];
#endif
        }

        ComponentType& GetComponent(const Entity& e) override
        {
            return m_storage[secs::eid_t(e.GetEID())];
        }

        void FreeComponent(const Entity& /*e*/) override
        {
            // VectorComponentStorage does not reclaim memory on remove;
            // the slot simply becomes stale until the entity is reused.
        }

        /**
         * @brief Returns bytes used by the flat storage vector (full capacity,
         *        including holes for unused entity slots).
         */
        size_t AllocatedBytes() const override
        {
            return m_storage.capacity() * sizeof(ComponentType);
        }

    private:
        std::vector<ComponentType> m_storage;
    };

    /**
     * @brief Alias for the component storage interface type used throughout the engine.
     *        ComponentManager holds SharedPtr<ComponentStorage<T>>.
     *
     * @tparam T The type of the component.
     */
    template <typename T>
    using ComponentStorage = TypedComponentStorage<T>;

}