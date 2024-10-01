#pragma once

#include <unordered_set>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <vector>

#include "entity.h"
#include "componentmanager.h"
#include "eidstorage.h"
#include "entityobserver.h"
#include "componentflagsmanager.h"

namespace secs
{

    /**
     * @brief The ComponentEdit class represents an edit operation on a component.
     */
    class ComponentEdit
    {
    public:

        /**
         * @brief The Type enum represents the type of component edit.
         */
        enum Type {
            RemoveComponent, ///< Indicates a component removal.
            AddComponent     ///< Indicates a component addition.
        };

        /**
         * @brief Constructs a ComponentEdit object.
         * @param entity The entity being edited.
         * @param type The type of edit.
         * @param index The index of the component type.
         */
        ComponentEdit(const Entity& entity, Type type, ctindex_t index);

        /**
         * @brief Gets the type of the component edit.
         * @return The type of the component edit.
         */
        Type GetType();

        /**
         * @brief Gets the index of the component type.
         * @return The index of the component type.
         */
        ctindex_t GetIndex();

        /**
         * @brief Gets the entity being edited.
         * @return The entity being edited.
         */
        const Entity& GetEntity();

    private:
        Entity m_entity; ///< The entity being edited.
        Type m_type; ///< The type of the component edit.
        ctindex_t m_index; ///< The index of the component type.

    };

    /**
     * @brief The EntityProcessor object is responsible for creating/removing entities and components.
     * The user must access this object in order to make any entity changes.
     */
    class EntityProcessor
    {
    public:

        friend class Engine;

        typedef EntityProcessor* Ptr;

        /**
         * @brief Constructs an EntityProcessor object.
         * @param system_manager The system manager observing the entities.
         * @param component_manager The component manager handling component storage.
         * @param component_flags_manager The component flags manager handling component flags.
         */
        EntityProcessor(EntityObserver& system_manager, ComponentManager& component_manager, ComponentFlagsManager& component_flags_manager);

        /**
         * @brief Adds a component of the specified type to the given entity.
         * @tparam ComponentType The type of the component to add.
         * @param entity The entity to add the component to.
         * @return A reference to the added component.
         */
        template <typename ComponentType>
        ComponentType& AddComponent(const Entity& entity)
        {
            assert(entity.IsValid());
            // assert(false == m_componentFlagsManager.HasComponent<ComponentType>(entity));
            return AddOrRetrieveComponent<ComponentType>(entity);
        }

        /**
         * @brief Adds or retrieves a component of the specified type for the given entity.
         * @tparam ComponentType The type of the component to add or retrieve.
         * @param entity The entity to add or retrieve the component for.
         * @return A reference to the added or retrieved component.
         */
        template <typename ComponentType>
        ComponentType& AddOrRetrieveComponent(const Entity& entity)
        {
            auto storage = m_componentManager.GetComponentStorageForComponentType<ComponentType>();
            if (false == m_componentFlagsManager.HasComponent<ComponentType>(entity))
            {
                ctindex_t component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();
                m_componentEdits.push_back(ComponentEdit(entity, ComponentEdit::Type::AddComponent, component_index));
                storage->AllocComponent(entity);
            }
            return storage->GetComponent(entity);
        }

        /**
         * @brief Removes a component of the specified type from the given entity.
         * @tparam ComponentType The type of the component to remove.
         * @param entity The entity to remove the component from.
         * @return A reference to the removed component.
         */
        template <typename ComponentType>
        ComponentType& RemoveComponent(const Entity& entity)
        {
            auto storage = m_componentManager.GetComponentStorageForComponentType<ComponentType>();
            if (true == m_componentFlagsManager.HasComponent<ComponentType>(entity))
            {
                m_componentEdits.push_back(ComponentEdit(entity, ComponentEdit::Type::RemoveComponent, ComponentTraits::GetIndexForComponentType<ComponentType>()));
            }
            return storage->GetComponent(entity);
        }

        /**
         * @brief Removes the given entity.
         * @param entity The entity to remove.
         */
        void RemoveEntity(const Entity& entity);

        /**
         * @brief Adds a new entity.
         * @return The newly added entity.
         */
        Entity AddEntity();

        /**
         * @brief Deactivates the given entity.
         * @param entity The entity to deactivate.
         */
        void DeactivateEntity(const Entity& entity)
        {
            m_deactivatedEntities.push_back(entity);
        }

        /**
         * @brief Gets a list of valid entities.
         * @return A vector of valid entity IDs.
         */
        std::vector<eid_t> ValidEntities()
        {
            return m_eidStorage.validEntities();
        }

        /**
         * @brief Gets the number of entities.
         * @return The number of entities.
         */
        int GetNumEntities();

        /**
         * @brief Gets a component of the specified type for the given entity.
         * @tparam ComponentType The type of the component to get.
         * @param e The entity to get the component for.
         * @return A reference to the component.
         */
        template <typename ComponentType>
        ComponentType& GetComponent(const secs::Entity& e)
        {
            return m_componentManager.GetComponentForEntity<ComponentType>(e);
        }

        /**
         * @brief Checks if the given entity has a component of the specified type.
         * @tparam ComponentType The type of the component to check for.
         * @param e The entity to check.
         * @return True if the entity has the component, false otherwise.
         */
        template <typename ComponentType>
        bool HasComponent(const secs::Entity& e)
        {
            return m_componentFlagsManager.HasComponent<ComponentType>(e);
        }

        /**
         * @brief Forces the application of changes to entities and components.
         */
        void ForceApplyChanges();

        /**
         * @brief Tags the given entity with the specified tag.
         * @param e The entity to tag.
         * @param tag The tag to assign to the entity.
         */
        void TagEntity(secs::Entity e, std::string tag)
        {
            if (m_taggedEntities.count(tag) == 0)
            {
                m_taggedEntities = {};
            }
            m_taggedEntities[tag].push_back(e);
        }

        /**
         * @brief Gets all entities with the specified tag.
         * @param tag The tag to search for.
         * @return A vector of entities with the specified tag.
         */
        std::vector<secs::Entity> GetAllEntitiesWithTag(const std::string& tag)
        {
            if (m_taggedEntities.count(tag) == 0)
            {
                return {};
            }
            return m_taggedEntities[tag];
        }

    private:

        /**
         * @brief Applies changes to entities and components.
         */
        void ApplyChanges();

        EntityObserver& m_systemManager; ///< The system manager observing the entities.
        ComponentManager& m_componentManager; ///< The component manager handling component storage.
        ComponentFlagsManager& m_componentFlagsManager; ///< The component flags manager handling component flags.
        EIDStorage m_eidStorage; ///< The storage for entity IDs.

        std::vector<Entity> m_addedEntities; ///< List of added entities.
        std::vector<Entity> m_removedEntities; ///< List of removed entities.
        std::vector<Entity> m_deactivatedEntities; ///< List of deactivated entities.
        std::vector<ComponentEdit> m_componentEdits; ///< List of component edits.

        std::unordered_map<std::string, std::vector<Entity>> m_taggedEntities; ///< Map of tagged entities.

        int m_numEntities; ///< The number of entities.

    };

}