
#include "entitysystem.h"

secs::EntitySystem::~EntitySystem()
{

}

void secs::EntitySystem::added(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		if( acceptsEntity( entity ) )
		{
			add( entity );
		}
	}
}

void secs::EntitySystem::removed(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		remove( entity );
	}
}

void secs::EntitySystem::changed(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		bool already_present = m_entityBits.test( entity.eid() );
		bool system_accept = acceptsEntity( entity );
		if( already_present && !system_accept )
		{
			remove( entity );
		}
		else if( !already_present && system_accept )
		{
			add( entity );
		}
	}
}

bool secs::EntitySystem::acceptsEntity(const secs::Entity& entity)
{
	ComponentBits &entity_component_bits = m_componentFlagsManager->componentFlags( entity );
	return ( m_neededComponents & entity_component_bits ) == m_neededComponents;
}

void secs::EntitySystem::onAdded(const secs::Entity &e)
{

}

void secs::EntitySystem::onRemoved(const secs::Entity &e)
{

}

void secs::EntitySystem::setComponentFlagsManager(secs::ComponentFlagsManager::Ptr cfm)
{
	m_componentFlagsManager = cfm;
}

void secs::EntitySystem::step()
{
	for( const Entity& e : m_activeEntities )
	{
		process( e );
	}
}

void secs::EntitySystem::remove(const secs::Entity &entity)
{
	m_entityBits.set( entity.eid(), false );
	m_activeEntities.erase( std::remove( m_activeEntities.begin(), m_activeEntities.end(), entity ), m_activeEntities.end() );
	onRemoved( entity );
}

void secs::EntitySystem::add(const secs::Entity &entity)
{
	m_entityBits.set( entity.eid(), true );
	m_activeEntities.push_back( entity );
	onAdded( entity );
}
