#pragma once

#include <vector>
#include <memory>

#include "componentstorage.h"
#include "componenttraits.h"

namespace secs {

    /**
     * @brief The ComponentManager class is a proxy between users and the
     * component storage for each component type. It owns every storage.
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
         *        The returned pointer is never null and stays valid for the lifetime of the
         *        ComponentManager, so callers may cache it.
         * @tparam ComponentType The type of the component.
         */
        template <typename ComponentType>
        TypedComponentStorage<ComponentType>* GetComponentStorageForComponentType()
        {
            const auto component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();

            if (component_index >= m_componentStorageVector.size())
            {
                m_componentStorageVector.resize(component_index + 1);
            }

            auto& slot = m_componentStorageVector[component_index];
            if (slot == nullptr)
            {
                if (m_storageMode == StorageMode::SparseSet)
                    slot = std::make_unique<SparseSetComponentStorage<ComponentType>>();
                else
                    slot = std::make_unique<VectorComponentStorage<ComponentType>>();
            }
            return static_cast<TypedComponentStorage<ComponentType>*>(slot.get());
        }

        /**
         * @brief Retrieves a specific component for a given entity.
         * @tparam ComponentType The type of the component.
         * @param e The entity for which the component is to be retrieved.
         * @return A reference to the component of the specified type for the given entity.
         */
        template <typename ComponentType>
        ComponentType& GetComponentForEntity(const Entity& e)
        {
            return GetComponentStorageForComponentType<ComponentType>()->GetComponent(e);
        }

        /**
         * @brief Frees the component of the given type index for the entity, if present.
         */
        void FreeComponent(const Entity& e, ctindex_t component_index)
        {
            if (component_index < m_componentStorageVector.size() && m_componentStorageVector[component_index] != nullptr)
            {
                m_componentStorageVector[component_index]->FreeIfPresent(e);
            }
        }

        /**
         * @brief Frees every component the entity has, releasing their resources.
         */
        void FreeAllComponents(const Entity& e)
        {
            for (auto& storage : m_componentStorageVector)
            {
                if (storage != nullptr)
                {
                    storage->FreeIfPresent(e);
                }
            }
        }

    private:
        StorageMode m_storageMode; ///< Backend selected at construction time.

        /**
         * @brief Storage for each component type, indexed by component type index.
         */
        std::vector<BaseComponentStorage::UniquePtr> m_componentStorageVector;

    };

}
