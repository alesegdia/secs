#pragma once

#include "componenttraits.h"
#include "bits.h"
#include "entity.h"

#include <vector>

namespace secs
{

    /**
     * @class ComponentFlagsManager
     * @brief Manages component flags for entities.
     */
    class ComponentFlagsManager
    {
    public:

        typedef ComponentFlagsManager* Ptr;

        /**
         * @brief Resets all component flags for a given entity, growing storage if needed.
         * @param entity The entity whose flags are to be reset.
         */
        void ResetFlagsForEntity(const Entity& entity);

        /**
         * @brief Sets a specific component flag for a given entity.
         * @param entity The entity whose component flag is to be set.
         * @param index The index of the component flag to set.
         */
        void SetComponentFlagForEntity(const Entity& entity, ctindex_t index);

        /**
         * @brief Clears a specific component flag for a given entity.
         * @param entity The entity whose component flag is to be cleared.
         * @param index The index of the component flag to clear.
         */
        void ClearComponentFlagForEntity(const Entity& entity, ctindex_t index);

        /**
         * @brief Checks if a given entity has a specific component.
         * @return True if the entity has the component, false otherwise (including
         *         for entities the manager has never seen).
         */
        template <typename ComponentType>
        bool HasComponent(const Entity& entity) const
        {
            return HasComponent(entity, ComponentTraits::GetIndexForComponentType<ComponentType>());
        }

        /**
         * @brief Checks if a given entity has the component with the given type index.
         */
        bool HasComponent(const Entity& entity, ctindex_t component_index) const
        {
            if (!entity.IsValid() || entity.Index() >= m_entityBits.size())
            {
                return false;
            }
            return m_entityBits[entity.Index()].Test(component_index);
        }

        /**
         * @brief Gets the component flags for a given entity.
         * @param entity The entity whose component flags are to be retrieved.
         * @return A reference to the component flags of the entity.
         */
        ComponentBits& GetComponentFlagsForEntity(const Entity& entity);

        /**
         * @brief Clears all component flags for a given entity.
         * @param entity The entity whose flags are to be cleared.
         */
        void ClearAllFlagsForEntity(const Entity& entity)
        {
            if (entity.Index() < m_entityBits.size())
            {
                m_entityBits[entity.Index()].Reset();
            }
        }

    private:
		// The component flags for each entity index.
        std::vector<ComponentBits> m_entityBits;

    };

}
