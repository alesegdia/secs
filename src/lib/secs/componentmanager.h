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
         * @brief Retrieves the component storage for a specific component type.
         * @tparam ComponentType The type of the component.
         * @return A shared pointer to the component storage for the specified component type.
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
                auto base_storage = std::shared_ptr<BaseComponentStorage>(new ComponentStorage<ComponentType>());
                component_storage = std::static_pointer_cast<ComponentStorage<ComponentType>>(base_storage);
                m_componentStorageVector[component_index] = base_storage;
            }
            else
            {
                auto base_storage = m_componentStorageVector[component_index];
                component_storage = std::static_pointer_cast<ComponentStorage<ComponentType>>(base_storage);
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
        /**
         * @brief A vector that stores shared pointers to the base component storage.
         */
        std::vector<BaseComponentStorage::SharedPtr> m_componentStorageVector;

    };

}