#pragma once

#include <algorithm>
#include <cassert>
#include <string>
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
        Type GetType() const;

        /**
         * @brief Gets the index of the component type.
         * @return The index of the component type.
         */
        ctindex_t GetIndex() const;

        /**
         * @brief Gets the entity being edited.
         * @return The entity being edited.
         */
        const Entity& GetEntity() const;

    private:
        Entity m_entity; ///< The entity being edited.
        Type m_type; ///< The type of the component edit.
        ctindex_t m_index; ///< The index of the component type.

    };

    /**
     * @brief The EntityProcessor object is responsible for creating/removing entities and components.
     * The user must access this object in order to make any entity changes.
     *
     * Component additions and removals take effect immediately for HasComponent/GetComponent.
     * Only the notification of systems (and the freeing of removed components) is deferred
     * until ApplyChanges, which runs at the start of Engine::Step. Changes made from system
     * callbacks during ApplyChanges are processed in the same ApplyChanges call.
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
         * @brief Adds a component of the specified type to the given entity. If the entity
         *        already has one, the existing component is returned unchanged.
         * @tparam ComponentType The type of the component to add.
         * @param entity The entity to add the component to.
         * @return A reference to the added component.
         */
        template <typename ComponentType>
        ComponentType& AddComponent(const Entity& entity)
        {
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
            assert(IsAlive(entity) && "AddComponent on a dead entity");
            auto* storage = m_componentManager.GetComponentStorageForComponentType<ComponentType>();
            const ctindex_t component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();
            if (!m_componentFlagsManager.HasComponent(entity, component_index))
            {
                m_componentFlagsManager.SetComponentFlagForEntity(entity, component_index);
                m_componentEdits.push_back(ComponentEdit(entity, ComponentEdit::Type::AddComponent, component_index));
                return storage->AllocComponent(entity);
            }
            return storage->GetComponent(entity);
        }

        /**
         * @brief Removes a component of the specified type from the given entity.
         *        HasComponent returns false immediately; the component data stays readable
         *        until systems have been notified in ApplyChanges, and is freed afterwards.
         * @tparam ComponentType The type of the component to remove.
         * @param entity The entity to remove the component from.
         * @return True if the entity had the component, false otherwise (no-op).
         */
        template <typename ComponentType>
        bool RemoveComponent(const Entity& entity)
        {
            const ctindex_t component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();
            if (!IsAlive(entity) || !m_componentFlagsManager.HasComponent(entity, component_index))
            {
                return false;
            }
            m_componentFlagsManager.ClearComponentFlagForEntity(entity, component_index);
            m_componentEdits.push_back(ComponentEdit(entity, ComponentEdit::Type::RemoveComponent, component_index));
            return true;
        }

        /**
         * @brief Removes the given entity. The entity stays alive (and its components
         *        readable) until systems have been notified in ApplyChanges.
         *        Removing a dead or already-removed entity does nothing.
         * @param entity The entity to remove.
         */
        void RemoveEntity(const Entity& entity);

        /**
         * @brief Adds a new entity.
         * @return The newly added entity.
         */
        Entity AddEntity();

        /**
         * @brief Checks whether the handle refers to a currently alive entity. Returns false
         *        for handles to removed entities, even if their index has been reused.
         */
        bool IsAlive(const Entity& entity) const
        {
            return m_eidStorage.IsAlive(entity);
        }

        /**
         * @brief Deactivates the given entity.
         * @param entity The entity to deactivate.
         */
        void DeactivateEntity(const Entity& entity)
        {
            m_deactivatedEntities.push_back(entity);
        }

        /**
         * @brief Gets the currently alive entities.
         */
        const std::vector<Entity>& ValidEntities() const
        {
            return m_eidStorage.validEntities();
        }

        /**
         * @brief Gets the number of entities, excluding those pending removal.
         * @return The number of entities.
         */
        int GetNumEntities() const;

        /**
         * @brief Gets a component of the specified type for the given entity.
         *        The entity must have the component (see TryGetComponent).
         * @tparam ComponentType The type of the component to get.
         * @param e The entity to get the component for.
         * @return A reference to the component.
         */
        template <typename ComponentType>
        ComponentType& GetComponent(const secs::Entity& e)
        {
            assert(IsAlive(e) && "GetComponent on a dead entity");
            return m_componentManager.GetComponentForEntity<ComponentType>(e);
        }

        /**
         * @brief Gets a component of the specified type, or nullptr if the entity is dead
         *        or does not have it.
         */
        template <typename ComponentType>
        ComponentType* TryGetComponent(const secs::Entity& e)
        {
            if (!HasComponent<ComponentType>(e))
            {
                return nullptr;
            }
            return &m_componentManager.GetComponentForEntity<ComponentType>(e);
        }

        /**
         * @brief Checks if the given entity has a component of the specified type.
         * @tparam ComponentType The type of the component to check for.
         * @param e The entity to check.
         * @return True if the entity is alive and has the component, false otherwise.
         */
        template <typename ComponentType>
        bool HasComponent(const secs::Entity& e) const
        {
            return IsAlive(e) && m_componentFlagsManager.HasComponent<ComponentType>(e);
        }

        /**
         * @brief Returns the typed component storage for a given component type.
         *        Useful for inspecting storage properties such as AllocatedBytes().
         *        The pointer is never null and is owned by the engine.
         */
        template <typename ComponentType>
        TypedComponentStorage<ComponentType>* GetComponentStorageForComponentType()
        {
            return m_componentManager.GetComponentStorageForComponentType<ComponentType>();
        }

        /**
         * @brief Forces the application of changes to entities and components.
         */
        void ForceApplyChanges();

        /**
         * @brief Tags the given entity with the specified tag. Tagging an entity twice
         *        with the same tag has no effect. Tags are dropped when the entity is removed.
         * @param e The entity to tag.
         * @param tag The tag to assign to the entity.
         */
        void TagEntity(const secs::Entity& e, const std::string& tag)
        {
            assert(IsAlive(e) && "TagEntity on a dead entity");
            auto& tagged = m_taggedEntities[tag];
            if (std::find(tagged.begin(), tagged.end(), e) == tagged.end())
            {
                tagged.push_back(e);
            }
        }

        /**
         * @brief Gets all entities with the specified tag.
         * @param tag The tag to search for.
         * @return The entities with the specified tag (empty if none).
         */
        const std::vector<secs::Entity>& GetAllEntitiesWithTag(const std::string& tag) const
        {
            static const std::vector<secs::Entity> empty;
            auto it = m_taggedEntities.find(tag);
            return it == m_taggedEntities.end() ? empty : it->second;
        }

    private:

        /**
         * @brief Applies changes to entities and components, repeating until changes made
         *        from system callbacks have been processed too.
         */
        void ApplyChanges();

        /**
         * @brief Removes the entity from every tag list.
         */
        void RemoveFromTags(const Entity& entity);

        bool IsPendingRemoval(const Entity& entity) const
        {
            return entity.Index() < m_pendingRemoval.size() && m_pendingRemoval[entity.Index()];
        }

        /// Upper bound on ApplyChanges passes, to catch callbacks that keep spawning changes forever.
        static constexpr int MaxApplyPasses = 64;

        EntityObserver& m_systemManager; ///< The system manager observing the entities.
        ComponentManager& m_componentManager; ///< The component manager handling component storage.
        ComponentFlagsManager& m_componentFlagsManager; ///< The component flags manager handling component flags.
        EIDStorage m_eidStorage; ///< The storage for entity IDs.

        std::vector<Entity> m_addedEntities; ///< List of added entities.
        std::vector<Entity> m_removedEntities; ///< List of removed entities.
        std::vector<Entity> m_deactivatedEntities; ///< List of deactivated entities.
        std::vector<ComponentEdit> m_componentEdits; ///< List of component edits.
        std::vector<bool> m_pendingRemoval; ///< Per entity index: removal requested but not applied yet.

        std::unordered_map<std::string, std::vector<Entity>> m_taggedEntities; ///< Map of tagged entities.

        int m_numEntities; ///< The number of entities, excluding those pending removal.

    };

}
