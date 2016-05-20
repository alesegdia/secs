#pragma once

#include <unordered_set>

#include "entity.h"
#include "componentmanager.h"
#include "eidstorage.h"
#include "systemmanager.h"

namespace secs
{

class ComponentEdit
{
public:

	enum Type {
		RemoveComponent,
		AddComponent
	};

	ComponentEdit( const Entity& entity, Type type, BaseComponentStorage::Ptr storage )
		: m_entity(entity), m_type(type), m_storage(storage)
	{

	}

	Type type()
	{
		return m_type;
	}

	BaseComponentStorage::Ptr storage()
	{
		return m_storage;
	}

	Entity entity()
	{
		return m_entity;
	}

private:
	Entity m_entity;
	Type m_type;
	BaseComponentStorage::Ptr m_storage;

};

class EntityProcessor
{
public:

	typedef EntityProcessor* Ptr;

	template <typename ComponentType>
	ComponentType& addComponent( const Entity& entity )
	{
		auto storage = m_componentManager.componentStorage<ComponentType>();
		m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::AddComponent, storage.data() ));
		storage->component( entity ) = {};
		return storage->component( entity );
	}

	template <typename ComponentType>
	void removeComponent( const Entity& entity )
	{
		auto storage = m_componentManager.componentStorage<ComponentType>();
		m_componentEdits.push_back( ComponentEdit( entity, ComponentEdit::Type::RemoveComponent, storage.data() ));
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

	void applyChanges()
	{
		std::list<Entity> change_list;
		for( ComponentEdit& edit : m_componentEdits )
		{
			change_list.push_back( edit.entity() );
			switch( edit.type() )
			{
			case ComponentEdit::Type::AddComponent:
				edit.storage()->setComponent( edit.entity() );
				break;
			case ComponentEdit::Type::RemoveComponent:
				edit.storage()->unsetComponent( edit.entity() );
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

private:

	ComponentManager m_componentManager;
	SystemManager m_systemManager;

	std::vector<Entity> m_addedEntities;
	std::vector<Entity> m_removedEntities;
	std::vector<ComponentEdit> m_componentEdits;
	EIDStorage m_eidStorage;

};

}
