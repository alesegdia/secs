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
    class VectorComponentStorage : public BaseComponentStorage
    {
    public:

        using UniquePtr = std::unique_ptr<VectorComponentStorage<ComponentType>>;
        using SharedPtr = std::shared_ptr<VectorComponentStorage>;
        using Ptr = VectorComponentStorage<ComponentType>*;

        /**
         * @brief Allocates and returns a component for the entity passed as a parameter.
         * Resizes the container if the entity does not fit.
         * @param e The entity for which the component is to be allocated.
         * @return The allocated component.
         */
        ComponentType& AllocComponent(const Entity& e)
        {
            if (m_storage.size() < (e.GetEID() + 1))
            {
                m_storage.resize(secs::eid_t(e.GetEID() + 32));
            }
            m_storage[secs::eid_t(e.GetEID())] = {};
            return m_storage[secs::eid_t(e.GetEID())];
        }

        /**
         * @brief Retrieves the component for a given entity.
         * @param e The entity for which the component is to be retrieved.
         * @return A reference to the component of the specified type for the given entity.
         */
        ComponentType& GetComponent(const Entity& e)
        {
            return m_storage[secs::eid_t(e.GetEID())];
        }

    private:

        /**
         * @brief Storage for components. The container is resized when a component is
         * requested for an entity and it does not fit.
         */
        std::vector<ComponentType> m_storage;

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
     * @brief Alias for the default component storage type.
     *
     * @tparam T The type of the component.
     */
    template <typename T>
    using ComponentStorage = VectorComponentStorage<T>;

}