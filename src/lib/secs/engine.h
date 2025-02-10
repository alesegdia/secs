#pragma once

#include <algorithm>
#include <functional>

#include "entity.h"
#include "systemmanager.h"
#include "componentmanager.h"
#include "entityprocessor.h"

namespace secs
{

    /**
     * @brief The Engine class is responsible for managing systems, entities, and components.
     */
    class Engine
    {
    public:

        typedef Engine* Ptr;
        using EntityVisitor = std::function<void(const secs::Entity& e)>;

        /**
         * @brief Constructs an Engine object.
         */
        Engine()
            : m_systemManager(m_componentFlagsManager),
            m_entityProcessor(m_systemManager, m_componentManager, m_componentFlagsManager)
        {

        }

        /**
         * @brief Visits all valid entities and applies the given visitor function.
         * @param v The visitor function to apply to each entity.
         */
        void Visit(EntityVisitor v)
        {
            for (auto e : GetEntityProcessor().ValidEntities())
            {
                v(Entity(e));
            }
        }

        /**
         * @brief Creates a system of type T and adds it to the engine.
         * @tparam T The type of the system to create.
         * @tparam Args The types of the arguments to pass to the system's constructor.
         * @param args The arguments to pass to the system's constructor.
         * @return A shared pointer to the created system.
         */
        template <typename T, typename... Args>
        std::shared_ptr<T> CreateSystem(Args&&... args)
        {
            auto sys = std::make_shared<T>(args...);
            PushSystem(sys);
            return sys;
        }

        /**
         * @brief Adds a system to the engine.
         * @param system The system to add.
         */
        void PushSystem(System::Ptr system)
        {
            EntitySystem::Ptr entity_system = std::dynamic_pointer_cast<EntitySystem>(system);
            if (nullptr != entity_system)
            {
                entity_system->SetComponentFlagsManager(&m_componentFlagsManager);
                entity_system->SetEntityProcessor(&m_entityProcessor);
                entity_system->SetComponentManager(&m_componentManager);
            }

            m_systemManager.PushSystem(system, entity_system);
        }

        /**
         * @brief Sets the system group for a given system.
         * @param system The system to set the group for.
         * @param group The group index to set.
         */
        void SetSystemGroup(System::Ptr system, SystemGroupIndex group)
        {
            m_systemManager.SetSystemGroup(system, group);
        }

        /**
         * @brief Disables a system group.
         * @param sgi The index of the system group to disable.
         */
        void DisableGroup(SystemGroupIndex sgi)
        {
            m_systemManager.DisableSystemGroup(sgi);
        }

        /**
         * @brief Enables a system group.
         * @param sgi The index of the system group to enable.
         */
        void EnableGroup(SystemGroupIndex sgi)
        {
            m_systemManager.EnableSystemGroup(sgi);
        }

        /**
         * @brief Activates a system group.
         * @param sgi The index of the system group to activate.
         */
        void ActivateSystemGroup(SystemGroupIndex sgi)
        {
            m_systemManager.ActivateSystemGroup(sgi);
        }

        /**
         * @brief Gets the entity processor.
         * @return A reference to the entity processor.
         */
        EntityProcessor& GetEntityProcessor()
        {
            return m_entityProcessor;
        }

        /**
         * @brief Gets a component of a given type for a given entity.
         * @tparam ComponentType The type of the component to get.
         * @param e The entity to get the component for.
         * @return A reference to the component.
         */
        template <typename ComponentType>
        ComponentType& GetComponent(const Entity& e)
        {
            return m_componentManager.GetComponentStorageForComponentType<ComponentType>()->GetComponent(e);
        }

        /**
         * @brief Adds a component of a given type to a given entity.
         * @tparam ComponentType The type of the component to add.
         * @param e The entity to add the component to.
         * @return A reference to the added component.
         */
        template <typename ComponentType>
        ComponentType& AddComponent(const Entity& e)
        {
            return GetEntityProcessor().AddComponent<ComponentType>(e);
        }

        /**
         * @brief Steps the engine, applying changes and updating systems.
         * @param delta The time delta to step by.
         */
        void Step(double delta)
        {
            m_entityProcessor.ApplyChanges();
            m_systemManager.Step(delta);
        }

        /**
         * @brief Renders the engine.
         */
        void Render()
        {
            m_systemManager.Render();
        }

        /**
         * @brief Deactivates a given entity.
         * @param e The entity to deactivate.
         */
        void Deactivate(const Entity e)
        {
            SECS_UNUSED(e);
        }

        /**
         * @brief Checks if a given entity has a component of a given type.
         * @tparam T The type of the component to check for.
         * @param e The entity to check.
         * @return True if the entity has the component, false otherwise.
         */
        template <typename T>
        bool HasComponent(const secs::Entity& e)
        {
            return m_componentFlagsManager.HasComponent<T>(e);
        }

		/**
		 * @brief Gets all systems in the engine.
		 * @return A vector of all systems in the engine.
		 */
		const std::vector<EntitySystem::Ptr>& GetAllSystems()
        {
            return m_systemManager.GetAllEntitySystems();
        }

    private:

        ComponentFlagsManager m_componentFlagsManager;
        SystemManager m_systemManager;
        ComponentManager m_componentManager;
        EntityProcessor m_entityProcessor;

    };

}