#pragma once

#include <unordered_set>
#include <algorithm>
#include <list>
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

	ComponentEdit( const Entity& entity, Type type, ctindex_t index )
		: m_entity(entity), m_type(type), m_index(index)
	{

	}

	Type type()
	{
		return m_type;
	}

	ctindex_t index()
	{
		return m_index;
	}

	const Entity& entity()
	{
		return m_entity;
	}

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

    EntityProcessor( EntityObserver& system_manager, ComponentManager& component_manager, ComponentFlagsManager& component_flags_manager )
	    : m_systemManager(system_manager),
		  m_componentManager(component_manager),
		  m_componentFlagsManager(component_flags_manager)
	{

	}

	template <typename ComponentType>
	ComponentType& addComponent( const Entity& entity )
	{
        auto storage = m_componentManager.componentStorage<ComponentType>();
        if( false == m_componentFlagsManager.hasComponent<ComponentType>(entity) )
        {
            ctindex_t component_index = ComponentTraits::getIndex<ComponentType>();
            m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::AddComponent, component_index ));
            storage->allocComponent( entity );
        }
		return storage->component( entity );
	}

	template <typename ComponentType>
	ComponentType& removeComponent( const Entity& entity )
	{
		auto storage = m_componentManager.componentStorage<ComponentType>();
        if( true == m_componentFlagsManager.hasComponent<ComponentType>(entity) )
        {
            m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::RemoveComponent, ComponentTraits::getIndex<ComponentType>() ));
        }
		return storage->component( entity );
	}

	void removeEntity( const Entity& entity )
	{
		m_removedEntities.push_back( entity );
	}

	Entity addEntity()
	{
		Entity entity = m_eidStorage.retrieve();
		m_addedEntities.push_back( entity );
		m_componentFlagsManager.reset( entity );
		return entity;
	}

	void deactivateEntity( const Entity& entity )
	{
		m_deactivatedEntities.push_back( entity );
	}

private:

	void applyChanges()
	{
		std::list<Entity> change_list;
		for( ComponentEdit& edit : m_componentEdits )
		{
			change_list.push_back( edit.entity() );
			switch( edit.type() )
			{
			case ComponentEdit::Type::AddComponent:
				m_componentFlagsManager.setComponentFlag( edit.entity(), edit.index() );
				break;
			case ComponentEdit::Type::RemoveComponent:
				m_componentFlagsManager.clearComponentFlag( edit.entity(), edit.index() );
				break;
			}
		}

		change_list.sort();
		change_list.unique();

		std::vector<Entity> change_vector { std::begin( change_list ), std::end( change_list ) } ;
		std::sort( change_vector.begin(), change_vector.end() );
		std::sort( m_addedEntities.begin(), m_addedEntities.end() );

		// we discard created entities from the change set because systems will already be notified
		// from m_addedEntities. We could remove this computation from here and let the system manager to
		// perform the difference computation, and pass all 3 sets without filtering at once
		// SystemManager::update( changed, added, removed )
		std::vector<Entity> filtered_change_vector;
		std::set_difference( change_vector.begin(), change_vector.end(),
							 m_addedEntities.begin(), m_addedEntities.end(),
							 std::back_inserter( filtered_change_vector ));

		m_systemManager.changed( filtered_change_vector );
		m_systemManager.added( m_addedEntities );
		m_systemManager.removed( m_removedEntities );

		for( auto entity : m_removedEntities )
		{
			m_eidStorage.recycle( entity );
			m_componentFlagsManager.clearAllFlags(entity);
		}

		m_addedEntities.clear();
		m_removedEntities.clear();
		m_deactivatedEntities.clear();
		m_componentEdits.clear();
	}

    EntityObserver& m_systemManager;
	ComponentManager& m_componentManager;
	ComponentFlagsManager& m_componentFlagsManager;
	EIDStorage m_eidStorage;

	std::vector<Entity> m_addedEntities;
	std::vector<Entity> m_removedEntities;
	std::vector<Entity> m_deactivatedEntities;
	std::vector<ComponentEdit> m_componentEdits;

};

}
