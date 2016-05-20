#pragma once

#include "entity.h"
#include "componentmanager.h"
#include "entitymanager.h"
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
	BaseComponentStorage::Ptr m_storage;
	Type m_type;
	Entity m_entity;

};

class EntityProcessor
{
public:

	template <typename ComponentType>
	ComponentType& addComponent( const Entity& entity )
	{
		auto storage = m_componentManager.componentStorage<ComponentType>();
		storage->setComponent(entity);

		ctindex_t component_index = ComponentTraits::getIndex<ComponentType>();
		m_componentEdits.push_back( ComponentEdit( ComponentEdit::Type::AddComponent, storage.data() ));
		storage->component( entity ) = {};
		return storage->component( entity );
	}

	void applyChanges()
	{
		for( ComponentEdit& edit : m_componentEdits )
		{
			switch( edit.type() )
			{
			case ComponentEdit::Type::AddComponent:
				edit.storage()->ownerEntityBits().set( edit.entity().eid(), true );
				break;
			case ComponentEdit::Type::RemoveComponent:
				edit.storage()->ownerEntityBits().set( edit.entity().eid(), false );
				break;
			}
		}
		m_componentManager.clear( m_removedEntities );

		m_systemManager.changed( ... );
		m_systemManager.added( m_addedEntities );
		m_systemManager.removed( m_removedEntities );
	}

private:
	ComponentManager m_componentManager;
	EntityManager m_entityManager;
	SystemManager m_systemManager;

	std::vector<Entity> m_addedEntities;
	std::vector<Entity> m_removedEntities;
	std::vector<ComponentEdit> m_componentEdits;

};

}
