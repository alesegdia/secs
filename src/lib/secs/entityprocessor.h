#pragma once

#include <unordered_set>

#include "entity.h"
#include "componentmanager.h"
#include "eidstorage.h"
#include "systemmanager.h"
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

	EntityProcessor( SystemManager& system_manager, ComponentManager& component_manager, ComponentFlagsManager& component_flags_manager )
		: m_systemManager(system_manager),
		  m_componentManager(component_manager),
		  m_componentFlagsManager(component_flags_manager)
	{

	}

	template <typename ComponentType>
	ComponentType& addComponent( const Entity& entity )
	{
		m_componentFlagsManager.componentFlags( entity );
		auto storage = m_componentManager.componentStorage<ComponentType>();
		m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::AddComponent, ComponentTraits::getIndex<ComponentType>() ));
		storage->component( entity ) = {};
		return storage->component( entity );
	}

	template <typename ComponentType>
	void removeComponent( const Entity& entity )
	{
		auto storage = m_componentManager.componentStorage<ComponentType>();
		m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::RemoveComponent, ComponentTraits::getIndex<ComponentType>() ));
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
		return entity;
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

		m_componentManager.clear( m_removedEntities );

		m_systemManager.changed( change_vector );
		m_systemManager.added( m_addedEntities );
		m_systemManager.removed( m_removedEntities );

		for( auto entity : m_removedEntities )
		{
			m_eidStorage.recycle( entity );
		}

		m_addedEntities.clear();
		m_removedEntities.clear();
	}

	SystemManager& m_systemManager;
	ComponentManager& m_componentManager;
	ComponentFlagsManager& m_componentFlagsManager;
	EIDStorage m_eidStorage;

	std::vector<Entity> m_addedEntities;
	std::vector<Entity> m_removedEntities;
	std::vector<ComponentEdit> m_componentEdits;

};

}
