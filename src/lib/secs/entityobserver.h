#pragma once

#include <vector>

#include "entity.h"

namespace secs
{

    /**
     * @brief The EntityObserver class is an abstract base class for observing changes to entities.
     */
    class EntityObserver
    {
    public:

        typedef EntityObserver* Ptr;

        /**
         * @brief Virtual destructor for EntityObserver.
         */
        virtual ~EntityObserver() = 0;

        /**
         * @brief Called when entities have changed.
         * @param entities The list of entities that have changed.
         */
        virtual void OnEntitiesChanged(const std::vector<Entity>& entities) = 0;

        /**
         * @brief Called when entities have been added.
         * @param entities The list of entities that have been added.
         */
        virtual void OnEntitiesAdded(const std::vector<Entity>& entities) = 0;

        /**
         * @brief Called when entities have been removed.
         * @param entities The list of entities that have been removed.
         */
        virtual void OnEntitiesRemoved(const std::vector<Entity>& entities) = 0;

    };

}