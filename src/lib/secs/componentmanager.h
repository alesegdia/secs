#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "componentstorage.h"
#include "componenttraits.h"

namespace secs {

    /**
     * @brief The ComponentManager class is a proxy between users and the
     * component storage for each component type.
     */
    class ComponentManager
    {
    public:

        typedef ComponentManager* Ptr;

        /**
         * @brief Constructs a ComponentManager with the specified storage backend.
         * @param mode Selects VectorComponentStorage or SparseSetComponentStorage.
         *             Defaults to SparseSet (packed, cache-friendly).
         */
        explicit ComponentManager(StorageMode mode = StorageMode::SparseSet)
            : m_storageMode(mode)
        {}

        /**
         * @brief Retrieves (or lazily creates) the component storage for a specific component type.
         * @tparam ComponentType The type of the component.
         * @return A shared pointer to the typed storage for the specified component type.
         */
        template <typename ComponentType>
        typename ComponentStorage<ComponentType>::SharedPtr GetComponentStorageForComponentType()
        {
            typename ComponentStorage<ComponentType>::SharedPtr component_storage;
            auto component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();

            if ((component_index + 1) > m_componentStorageVector.size())
            {
                m_componentStorageVector.resize(component_index + 1);
            }

            if (m_componentStorageVector[component_index] == nullptr)
            {
                std::shared_ptr<TypedComponentStorage<ComponentType>> typed_storage;
                if (m_storageMode == StorageMode::SparseSet)
                    typed_storage = std::make_shared<SparseSetComponentStorage<ComponentType>>();
                else
                    typed_storage = std::make_shared<VectorComponentStorage<ComponentType>>();

                m_componentStorageVector[component_index] = typed_storage;
                component_storage = typed_storage;
            }
            else
            {
                component_storage = std::static_pointer_cast<TypedComponentStorage<ComponentType>>(
                    m_componentStorageVector[component_index]);
            }
            return component_storage;
        }

        /**
         * @brief Retrieves a specific component for a given entity.
         * @tparam ComponentType The type of the component.
         * @param e The entity for which the component is to be retrieved.
         * @return A reference to the component of the specified type for the given entity.
         */
        template <typename ComponentType>
        ComponentType& GetComponentForEntity(Entity e)
        {
            return GetComponentStorageForComponentType<ComponentType>()->GetComponent(e);
        }

    private:
        StorageMode m_storageMode; ///< Backend selected at construction time.

        /**
         * @brief A vector that stores shared pointers to the base component storage.
         */
        std::vector<BaseComponentStorage::SharedPtr> m_componentStorageVector;

    };

}