#pragma once

#include <stack>
#include <cstdlib>
#include <vector>

#include "entity.h"

namespace secs
{

    /**
     * @class EIDStorage
     * @brief Manages the storage and recycling of entity IDs.
     */
    class EIDStorage
    {
    public:

        /**
         * @brief Retrieves a new entity ID.
         * @return The new entity ID.
         */
        eid_t Retrieve();

        /**
         * @brief Recycles an entity ID, making it available for reuse.
         * @param entity The entity whose ID is to be recycled.
         */
        void Recycle(Entity entity);

        /**
         * @brief Gets the last assigned entity ID.
         * @return The last assigned entity ID.
         */
        eid_t GetLastEID() const;

        /**
         * @brief Gets the number of recycled entity IDs.
         * @return The number of recycled entity IDs.
         */
        size_t GetNumRecycledEIDs() const;

        /**
         * @brief Gets the stack of recycled entity IDs.
         * @return A reference to the stack of recycled entity IDs.
         */
        const std::stack<eid_t>& GetRecycledStack() const;

        /**
         * @brief Gets a vector of valid entity IDs.
         * @return A vector of valid entity IDs.
         */
        std::vector<eid_t> validEntities()
        {
            return m_validEntities;
        }

    private:
        std::stack<eid_t> m_recycledEIDs; ///< Stack of recycled entity IDs.
        std::vector<eid_t> m_validEntities; ///< Vector of valid entity IDs.
        eid_t m_lastEID = 0; ///< The last assigned entity ID.

    };

}