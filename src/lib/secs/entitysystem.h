#pragma once

#include <list>
#include <algorithm>
#include <iostream>

#include "entityobserver.h"
#include "system.h"
#include "bits.h"
#include "componenttraits.h"
#include "componentflagsmanager.h"
#include "componentmanager.h"
#include "entityprocessor.h"

namespace secs
{

    /**
     * @brief The EntitySystem class is responsible for managing entities and their components.
     */
    class EntitySystem : public System, public EntityObserver
    {
    public:

        typedef std::shared_ptr<EntitySystem> Ptr;

        /**
         * @brief Sets the needed components for the system.
         * @tparam Args The types of the needed components.
         */
        template <typename... Args>
        void SetNeededComponents()
        {
            m_neededComponents = ComponentBitsBuilder<Args...>::BuildBits();
        }

        /**
         * @brief Adds needed components to the system.
         * @tparam Args The types of the needed components.
         */
        template <typename... Args>
        void AddNeededComponents()
        {
            m_neededComponents |= ComponentBitsBuilder<Args...>::BuildBits();
        }

        /**
         * @brief Virtual destructor for EntitySystem.
         */
        ~EntitySystem() = 0;

        // System interface
        /**
         * @brief Steps the system by the given delta time.
         * @param delta The delta time.
         */
        void Step(double delta) final;

        /**
         * @brief Renders the system.
         */
        void RenderStep() override;

        // EntityObserver interface
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
         * @brief Called when entities have changed.
         * @param entities The list of entities that have changed.
         */
        void OnEntitiesChanged(const std::vector<Entity>& entities) final;

        /**
         * @brief Checks if the system accepts the given entity.
         * @param entity The entity to check.
         * @return True if the system accepts the entity, false otherwise.
         */
        virtual bool AcceptsEntity(const Entity& entity);

        /**
         * @brief Called when an entity is added to the system.
         * @param e The entity that was added.
         */
        virtual void OnEntityAdded(const Entity& e);

        /**
         * @brief Called when an entity is removed from the system.
         * @param e The entity that was removed.
         */
        virtual void OnEntityRemoved(const Entity& e);

        /**
         * @brief Processes the given entity.
         * @param delta The delta time.
         * @param e The entity to process.
         */
        virtual void Process(double delta, const Entity& e);

        /**
         * @brief Renders the given entity.
         * @param e The entity to render.
         */
        virtual void Render(const Entity& e);

        /**
         * @brief Sets the component flags manager.
         * @param cfm The component flags manager.
         */
        void SetComponentFlagsManager(ComponentFlagsManager::Ptr cfm);

        /**
         * @brief Sets the entity processor.
         * @param processor The entity processor.
         */
        void SetEntityProcessor(EntityProcessor::Ptr processor)
        {
            m_entityProcessor = processor;
        }

        /**
         * @brief Sets the component manager.
         * @param component_manager The component manager.
         */
        void SetComponentManager(ComponentManager::Ptr component_manager)
        {
            m_componentManager = component_manager;
        }

        /**
         * @brief Adds a component of the specified type to the given entity.
         * @tparam ComponentType The type of the component to add.
         * @param e The entity to add the component to.
         * @return A reference to the added component.
         */
        template <typename ComponentType>
        ComponentType& AddComponent(const secs::Entity& e)
        {
            return m_entityProcessor->AddComponent<ComponentType>(e);
        }

        /**
         * @brief Removes a component of the specified type from the given entity.
         * @tparam ComponentType The type of the component to remove.
         * @param e The entity to remove the component from.
         * @return A reference to the removed component.
         */
        template <typename ComponentType>
        ComponentType& RemoveComponent(const secs::Entity& e)
        {
            return m_entityProcessor->RemoveComponent<ComponentType>(e);
        }

    protected:
        /**
         * @brief Gets the entity processor.
         * @return The entity processor.
         */
        EntityProcessor::Ptr GetEntityProcessor();

        /**
         * @brief Gets a component of the specified type for the given entity.
         * @tparam ComponentType The type of the component to get.
         * @param e The entity to get the component for.
         * @return A reference to the component.
         */
        template <typename ComponentType>
        ComponentType& GetComponent(const Entity& e)
        {
            return m_componentManager->GetComponentForEntity<ComponentType>(e);
        }

        /**
         * @brief Checks if the given entity has a component of the specified type.
         * @tparam ComponentType The type of the component to check for.
         * @param e The entity to check.
         * @return True if the entity has the component, false otherwise.
         */
        template <typename ComponentType>
        bool HasComponent(const Entity& e)
        {
            return m_componentFlagsManager->HasComponent<ComponentType>(e);
        }

        /**
         * @brief Destroys the given entity.
         * @param entity The entity to destroy.
         */
        void DestroyEntity(const Entity& entity)
        {
            GetEntityProcessor()->RemoveEntity(entity);
        }

    private:
        /**
         * @brief Removes the given entity.
         * @param entity The entity to remove.
         */
        void RemoveEntity(const Entity& entity);

        /**
         * @brief Adds the given entity.
         * @param entity The entity to add.
         */
        void AddEntity(const Entity& entity);

        ComponentBits m_neededComponents; ///< The needed components for the system.
        EntityBits m_entityBits; ///< The entity bits.
        std::list<Entity> m_activeEntities; ///< The list of active entities.
        ComponentFlagsManager::Ptr m_componentFlagsManager = nullptr; ///< The component flags manager.
        EntityProcessor::Ptr m_entityProcessor = nullptr; ///< The entity processor.
        ComponentManager::Ptr m_componentManager = nullptr; ///< The component manager.

    };

    /**
     * @brief This class represents an entity system which has automatic component deduction.
     * @tparam Args The types of the components.
     */
    template <typename... Args>
    class TypedEntitySystem : public EntitySystem
    {
    public:

        typedef TypedEntitySystem<Args...> ParentType;

        /**
         * @brief Constructs a TypedEntitySystem object.
         */
        TypedEntitySystem()
        {
            SetNeededComponents<Args...>();
        }

        /**
         * @brief Virtual destructor for TypedEntitySystem.
         */
        virtual ~TypedEntitySystem()
        {

        }

        /**
         * @brief Processes the given entity.
         * @param delta The delta time.
         * @param e The entity to process.
         */
        void Process(double delta, const secs::Entity& e) final
        {
            process(delta, e, GetComponent<Args>(e)...);
        }

        /**
         * @brief Processes the given entity with the specified components.
         * @param delta The delta time.
         * @param e The entity to process.
         * @param args The components of the entity.
         */
        virtual void process(double delta, const secs::Entity& e, Args&... args) = 0;

    private:
        ComponentManager::Ptr m_componentManager; ///< The component manager.

    };

}