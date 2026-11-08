#pragma once

#include <cstdlib>
#include <vector>

#include "entity.h"

namespace secs
{

    /**
     * @class EIDStorage
     * @brief Manages the storage and recycling of entity IDs.
     *
     * Each index has a generation counter that is bumped when the index is recycled,
     * so handles to removed entities can be told apart from the entity that reuses
     * the index (see IsAlive()).
     */
    class EIDStorage
    {
    public:

        /**
         * @brief Retrieves a new entity (recycled index with a new generation, or a fresh index).
         * @return The new entity.
         */
        Entity Retrieve();

        /**
         * @brief Recycles an entity's index, making it available for reuse.
         * Does nothing if the entity is not alive.
         * @param entity The entity whose ID is to be recycled.
         */
        void Recycle(const Entity& entity);

        /**
         * @brief Checks whether a handle refers to a currently alive entity.
         */
        bool IsAlive(const Entity& entity) const;

        /**
         * @brief Gets the number of indices handed out so far (alive or recycled).
         */
        eid_t GetLastEID() const;

        /**
         * @brief Gets the number of recycled entity indices waiting for reuse.
         */
        size_t GetNumRecycledEIDs() const;

        /**
         * @brief Gets the currently alive entities.
         */
        const std::vector<Entity>& validEntities() const
        {
            return m_validEntities;
        }

    private:
        std::vector<Entity::index_t> m_recycledIndices; ///< Free list of recycled indices (used as a stack).
        std::vector<Entity::generation_t> m_generations; ///< Current generation for each index.
        std::vector<bool> m_alive; ///< Whether each index currently holds a live entity.
        std::vector<Entity> m_validEntities; ///< Alive entities.

    };

}
