#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

#include "entity.h"

namespace secs {

    /**
     * @class BaseComponentStorage
     * @brief Type-erased base class for component storage.
     *
     * Lets the engine free every component of a removed entity without knowing
     * the component types.
     */
    class BaseComponentStorage {
    public:

        typedef std::unique_ptr<BaseComponentStorage> UniquePtr;
        typedef BaseComponentStorage* Ptr;

        virtual ~BaseComponentStorage() {}

        /**
         * @brief Returns true if a component slot is allocated for the entity.
         */
        virtual bool Contains(const Entity& e) const = 0;

        /**
         * @brief Frees the component slot for the entity, if any, releasing the
         *        component's resources.
         */
        virtual void FreeIfPresent(const Entity& e) = 0;

    };

    /**
     * @brief Selects which concrete storage backend is created by ComponentManager.
     */
    enum class StorageMode
    {
        Vector,    ///< VectorComponentStorage — storage indexed directly by entity index
        SparseSet  ///< SparseSetComponentStorage — packed dense array + sparse index (default)
    };

    /**
     * @brief Number of components per storage page. Components live in fixed-size
     *        pages that are never reallocated, so adding components never moves
     *        existing ones in memory.
     */
    static constexpr size_t ComponentPageSize = 256;

    /**
     * @brief Pure-virtual typed base that unifies VectorComponentStorage and
     * SparseSetComponentStorage behind a single interface used by ComponentManager.
     *
     * Reference stability: a reference returned by AllocComponent/GetComponent stays
     * valid while other components of the same type are added. Freeing a component
     * may move another one (SparseSet swaps the last element into the hole), so do
     * not hold references across ApplyChanges.
     *
     * @tparam ComponentType The type of the component.
     */
    template <typename ComponentType>
    class TypedComponentStorage : public BaseComponentStorage
    {
    public:
        using UniquePtr = std::unique_ptr<TypedComponentStorage<ComponentType>>;
        using Ptr       = TypedComponentStorage<ComponentType>*;

        virtual ComponentType& AllocComponent(const Entity& e) = 0;
        virtual ComponentType& GetComponent(const Entity& e)   = 0;
        virtual void           FreeComponent(const Entity& e)  = 0;

        void FreeIfPresent(const Entity& e) override
        {
            if (this->Contains(e))
            {
                FreeComponent(e);
            }
        }

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
     *   m_sparse[index] → position in the dense sequence (one array lookup)
     *   dense[position] → the actual ComponentType value, stored in fixed-size pages
     *
     * Properties:
     *   - O(1) random access via entity index (1 indirection through the sparse array)
     *   - Components are tightly packed (no holes) in pages of ComponentPageSize
     *   - Pages never move, so adding components never invalidates references
     *   - FreeComponent moves the last dense element into the freed slot so the
     *     dense sequence stays packed.
     *
     * @tparam ComponentType The type of the component.
     */
    template <typename ComponentType>
    class SparseSetComponentStorage final : public TypedComponentStorage<ComponentType>
    {
    public:
        using UniquePtr = std::unique_ptr<SparseSetComponentStorage<ComponentType>>;
        using Ptr       = SparseSetComponentStorage<ComponentType>*;

        static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

        bool Contains(const Entity& e) const override
        {
            const size_t index = e.Index();
            return index < m_sparse.size() && m_sparse[index] != InvalidIndex;
        }

        /**
         * @brief Allocates (or resets) a component slot for the given entity.
         */
        ComponentType& AllocComponent(const Entity& e) override
        {
            const size_t index = e.Index();
            GrowSparse(index);

            if (m_sparse[index] == InvalidIndex)
            {
                if (m_size == m_pages.size() * ComponentPageSize)
                {
                    m_pages.push_back(std::make_unique<ComponentType[]>(ComponentPageSize));
                }
                m_sparse[index] = m_size;
                m_denseEntities.push_back(e);
                m_size++;
            }
            ComponentType& slot = Dense(m_sparse[index]);
            slot = ComponentType{};
            return slot;
        }

        /**
         * @brief Retrieves the component for a given entity.
         *        One indirection: sparse[index] → dense[position].
         */
        ComponentType& GetComponent(const Entity& e) override
        {
            assert(Contains(e) && "Entity has no component of this type");
            return Dense(m_sparse[e.Index()]);
        }

        /**
         * @brief Frees the component slot for the given entity.
         *        Moves the last element into the hole to keep the dense array packed,
         *        and resets the vacated slot so its resources are released.
         */
        void FreeComponent(const Entity& e) override
        {
            assert(Contains(e));
            const size_t index    = e.Index();
            const size_t denseIdx = m_sparse[index];
            const size_t lastIdx  = m_size - 1;

            if (denseIdx != lastIdx)
            {
                Dense(denseIdx)           = std::move(Dense(lastIdx));
                m_denseEntities[denseIdx] = m_denseEntities[lastIdx];
                m_sparse[m_denseEntities[denseIdx].Index()] = denseIdx;
            }

            Dense(lastIdx) = ComponentType{};
            m_denseEntities.pop_back();
            m_size--;
            m_sparse[index] = InvalidIndex;
        }

        /**
         * @brief Returns bytes used by the dense component pages (capacity).
         *        Does not include the sparse index or entity list overhead.
         */
        size_t AllocatedBytes() const override
        {
            return m_pages.size() * ComponentPageSize * sizeof(ComponentType);
        }

    private:
        ComponentType& Dense(size_t position)
        {
            return m_pages[position / ComponentPageSize][position % ComponentPageSize];
        }

        void GrowSparse(size_t index)
        {
            if (index >= m_sparse.size())
                m_sparse.resize(index + 1, InvalidIndex);
        }

        std::vector<size_t>                           m_sparse;        ///< sparse[index] → position in dense
        std::vector<std::unique_ptr<ComponentType[]>> m_pages;         ///< dense component pages (never move)
        std::vector<Entity>                           m_denseEntities; ///< entity at each dense position
        size_t                                        m_size = 0;      ///< number of live components
    };

    /**
     * @brief Storage indexed directly by entity index.
     *
     * Components live in pages of ComponentPageSize addressed by entity index; a page
     * is allocated the first time an entity in its range gets a component. Pages never
     * move, so references stay valid while other components are added or removed.
     *
     * @tparam ComponentType The type of the component.
     */
    template <typename ComponentType>
    class VectorComponentStorage final : public TypedComponentStorage<ComponentType>
    {
    public:
        using UniquePtr = std::unique_ptr<VectorComponentStorage<ComponentType>>;
        using Ptr       = VectorComponentStorage<ComponentType>*;

        bool Contains(const Entity& e) const override
        {
            const size_t index = e.Index();
            return index < m_present.size() && m_present[index];
        }

        ComponentType& AllocComponent(const Entity& e) override
        {
            const size_t index = e.Index();
            const size_t page  = index / ComponentPageSize;
            if (page >= m_pages.size())
            {
                m_pages.resize(page + 1);
            }
            if (m_pages[page] == nullptr)
            {
                m_pages[page] = std::make_unique<ComponentType[]>(ComponentPageSize);
            }
            if (index >= m_present.size())
            {
                m_present.resize(index + 1, false);
            }
            m_present[index] = true;

            ComponentType& slot = Slot(index);
            slot = ComponentType{};
            return slot;
        }

        ComponentType& GetComponent(const Entity& e) override
        {
            assert(Contains(e) && "Entity has no component of this type");
            return Slot(e.Index());
        }

        /**
         * @brief Resets the slot so the component's resources are released. The page
         *        itself is kept for reuse.
         */
        void FreeComponent(const Entity& e) override
        {
            assert(Contains(e));
            const size_t index = e.Index();
            Slot(index) = ComponentType{};
            m_present[index] = false;
        }

        /**
         * @brief Returns bytes used by the allocated pages (including unused slots).
         */
        size_t AllocatedBytes() const override
        {
            size_t pages = 0;
            for (const auto& p : m_pages)
            {
                if (p != nullptr) pages++;
            }
            return pages * ComponentPageSize * sizeof(ComponentType);
        }

    private:
        ComponentType& Slot(size_t index)
        {
            return m_pages[index / ComponentPageSize][index % ComponentPageSize];
        }

        std::vector<std::unique_ptr<ComponentType[]>> m_pages;   ///< pages addressed by index / ComponentPageSize
        std::vector<bool>                             m_present; ///< whether each index holds a component
    };

    /**
     * @brief Alias for the component storage interface type used throughout the engine.
     *
     * @tparam T The type of the component.
     */
    template <typename T>
    using ComponentStorage = TypedComponentStorage<T>;

}
