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
         * @brief Resets all component flags for a given entity.
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
         * @tparam ComponentType The type of the component to check.
         * @param entity The entity to check.
         * @return True if the entity has the component, false otherwise.
         */
        template <typename ComponentType>
        bool HasComponent(const Entity& entity)
        {
            ctindex_t component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();
            return GetComponentFlagsForEntity(entity)[component_index];
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
            m_entityBits[secs::eid_t(entity.GetEID())].reset();
        }

    private:
		// The component flags for each entity.
        std::vector<ComponentBits> m_entityBits;

    };

}