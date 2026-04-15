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
         * @brief Sets the needed components for an entity match the system.
         * @tparam Args The types of the needed components.
         */
        template <typename... Args>
        void SetNeededComponents()
        {
            m_neededComponents = ComponentBitsBuilder<Args...>::BuildBits();
        }

        /**
         * @brief Adds needed components for an entity match the system.
         * @tparam Args The types of the needed components.
         */
        template <typename... Args>
        void AddNeededComponents()
        {
            m_neededComponents |= ComponentBitsBuilder<Args...>::BuildBits();
        }

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
        void OnEntitiesAdded(const std::vector<Entity>& entities) final;
        void OnEntitiesRemoved(const std::vector<Entity>& entities) final;
        void OnEntitiesChanged(const std::vector<Entity>& entities) final;

        // Public interface of EntitySystem
        virtual bool AcceptsEntity(const Entity& entity);
        virtual void OnEntityAdded(const Entity& e);
        virtual void OnEntityRemoved(const Entity& e);
        virtual void Process(double delta, const Entity& e);
        virtual void Render(const Entity& e);

        void SetComponentFlagsManager(ComponentFlagsManager::Ptr cfm);
        void SetEntityProcessor(EntityProcessor::Ptr processor)
        {
            m_entityProcessor = processor;
        }
        void SetComponentManager(ComponentManager::Ptr component_manager)
        {
            m_componentManager = component_manager;
        }

        template <typename ComponentType>
        ComponentType& AddComponent(const secs::Entity& e)
        {
            return m_entityProcessor->AddComponent<ComponentType>(e);
        }

        template <typename ComponentType>
        ComponentType& RemoveComponent(const secs::Entity& e)
        {
            return m_entityProcessor->RemoveComponent<ComponentType>(e);
        }

    protected:
        EntityProcessor::Ptr GetEntityProcessor();

        template <typename ComponentType>
        ComponentType& GetComponent(const Entity& e)
        {
            return m_componentManager->GetComponentForEntity<ComponentType>(e);
        }

        template <typename ComponentType>
        bool HasComponent(const Entity& e) const
        {
            return m_componentFlagsManager->HasComponent<ComponentType>(e);
        }

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
            Process(delta, e, GetComponent<Args>(e)...);
        }

        /**
         * @brief Processes the given entity with the specified components.
         * @param delta The delta time.
         * @param e The entity to process.
         * @param args The components of the entity.
         */
        virtual void Process(double delta, const secs::Entity& e, Args&... args) = 0;

    private:
        ComponentManager::Ptr m_componentManager; ///< The component manager.

    };

}