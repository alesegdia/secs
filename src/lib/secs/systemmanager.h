#pragma once

#include "entityobserver.h"
#include "system.h"
#include "entitysystem.h"
#include "componentflagsmanager.h"

#include <unordered_map>

namespace secs
{

    typedef size_t SystemGroupIndex;

    /**
     * @brief The SystemManager class manages systems and their execution in the ECS (Entity Component System).
     */
    class SystemManager : public EntityObserver
    {
    public:

        static constexpr SystemGroupIndex MaxSystemGroups = 32;

        /**
         * @brief Constructs a SystemManager with the given ComponentFlagsManager.
         * @param flags_manager The ComponentFlagsManager to use.
         */
        SystemManager(ComponentFlagsManager& flags_manager);

        /**
         * @brief Destructor for SystemManager.
         */
        ~SystemManager();

        // EntityObserver interface
        /**
         * @brief Called when entities have changed.
         * @param entities The list of entities that have changed.
         */
        void OnEntitiesChanged(const std::vector<Entity>& entities) final;

        /**
         * @brief Called when entities have been added.
         * @param entities The list of entities that have been added.
         */
        void OnEntitiesAdded(const std::vector<Entity>& entities) final;

        /**
         * @brief Called when entities have been removed.
         * @param entities The list of entities that have been removed.
         */
        void OnEntitiesRemoved(const std::vector<Entity>& entities) final;

        /**
         * @brief Pushes a system to the manager.
         * @param system The system to push.
         * @param entity_system The entity system associated with the system.
         */
        void PushSystem(secs::System::Ptr system, EntitySystem::Ptr entity_system);

        /**
         * @brief Activates a system group.
         * @param sgi The index of the system group to activate.
         */
        void ActivateSystemGroup(SystemGroupIndex sgi);

        /**
         * @brief Sets the system group for a system.
         * @param system The system to set the group for.
         * @param group The index of the group to set.
         */
        void SetSystemGroup(System::Ptr system, SystemGroupIndex group);

        /**
         * @brief Disables a system group.
         * @param sgi The index of the system group to disable.
         */
        void DisableSystemGroup(SystemGroupIndex sgi);

        /**
         * @brief Enables a system group.
         * @param sgi The index of the system group to enable.
         */
        void EnableSystemGroup(SystemGroupIndex sgi);

        /**
         * @brief Steps all systems by the given delta time.
         * @param delta The delta time.
         */
        void Step(double delta);

        /**
         * @brief Renders all systems.
         */
        void Render();

        const std::vector<EntitySystem::Ptr>& GetAllEntitySystems() const
        {
            return m_entitySystems;
        }

    private:

        /**
         * @brief Inserts a system into a sorted vector.
         * @param system The system to insert.
         * @param vec The vector to insert the system into.
         */
        void insertSorted(System::Ptr system, std::vector<System::Ptr>& vec);

        /**
         * @brief All systems to execute on step().
         */
        std::vector<System::Ptr> m_processingSystems;

        /**
         * @brief All rendering systems to execute on render().
         */
        std::vector<System::Ptr> m_renderingSystems;

        /**
         * @brief All entity systems for entity change/add/removal notification.
         */
        std::vector<EntitySystem::Ptr> m_entitySystems;

        ComponentFlagsManager& m_flagsManager; ///< The component flags manager.

        std::vector<System::Ptr>* m_systemsByGroup[MaxSystemGroups]{}; ///< Systems grouped by index.

    };

}