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

class ComponentEdit
{
public:

	enum Type {
		RemoveComponent,
		AddComponent
	};

	ComponentEdit( const Entity& entity, Type type, ctindex_t index );

	Type GetType();

	ctindex_t GetIndex();

	const Entity& GetEntity();

private:
	Entity m_entity;
	Type m_type;
	ctindex_t m_index;

};

/**
 * @brief The EntityProcessor object is the responsible of creating/removing entities and components.
 * The user must access this object in order to make any entity changes.
 */
class EntityProcessor
{
public:

	friend class Engine;

	typedef EntityProcessor* Ptr;

    EntityProcessor( EntityObserver& system_manager, ComponentManager& component_manager, ComponentFlagsManager& component_flags_manager );

    template <typename ComponentType>
    ComponentType& AddComponent( const Entity& entity )
    {
		assert(entity.IsValid());
        assert(false == m_componentFlagsManager.HasComponent<ComponentType>(entity));
        return AddOrRetrieveComponent<ComponentType>(entity);
    }

    template <typename ComponentType>
    ComponentType& AddOrRetrieveComponent( const Entity& entity )
    {
        auto storage = m_componentManager.GetComponentStorageForComponentType<ComponentType>();
        if( false == m_componentFlagsManager.HasComponent<ComponentType>(entity) )
        {
            ctindex_t component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();
            m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::AddComponent, component_index ));
            storage->AllocComponent( entity );
        }
        return storage->GetComponent( entity );
    }

    template <typename ComponentType>
	ComponentType& RemoveComponent( const Entity& entity )
	{
		auto storage = m_componentManager.GetComponentStorageForComponentType<ComponentType>();
        if( true == m_componentFlagsManager.HasComponent<ComponentType>(entity) )
        {
            m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::RemoveComponent, ComponentTraits::GetIndexForComponentType<ComponentType>() ));
        }
		return storage->GetComponent( entity );
	}

	void RemoveEntity( const Entity& entity );

	Entity AddEntity();

	void DeactivateEntity( const Entity& entity )
	{
		m_deactivatedEntities.push_back( entity );
	}

	std::vector<eid_t> ValidEntities()
	{
		return m_eidStorage.validEntities();
	}

    int GetNumEntities();

    template <typename ComponentType>
    ComponentType& Component(const secs::Entity& e)
    {
        return m_componentManager.GetComponentForEntity<ComponentType>(e);
    }

    template <typename ComponentType>
    bool HasComponent(const secs::Entity& e)
    {
        return m_componentFlagsManager.HasComponent<ComponentType>(e);
    }

	void ForceApplyChanges();

	void TagEntity(secs::Entity e, std::string tag)
	{
		if(m_taggedEntities.count(tag) == 0)
		{
			m_taggedEntities = {};
		}
		m_taggedEntities[tag].push_back(e);
	}

	std::vector<secs::Entity> GetAllEntitiesWithTag(const std::string& tag)
	{
		if(m_taggedEntities.count(tag) == 0)
		{
			return {};
		}
		return m_taggedEntities[tag];
	}

private:

	void ApplyChanges();

    EntityObserver& m_systemManager;
	ComponentManager& m_componentManager;
	ComponentFlagsManager& m_componentFlagsManager;
	EIDStorage m_eidStorage;

	std::vector<Entity> m_addedEntities;
	std::vector<Entity> m_removedEntities;
	std::vector<Entity> m_deactivatedEntities;
	std::vector<ComponentEdit> m_componentEdits;

	std::unordered_map<std::string, std::vector<Entity>> m_taggedEntities;

    int m_numEntities;

};

}
