#include "systemmanager.h"

secs::SystemManager::SystemManager(secs::ComponentFlagsManager &flags_manager)
	: m_flagsManager( flags_manager )
{

}

void secs::SystemManager::changed(const std::vector<secs::Entity> &entities)
{
	for( auto system : m_entitySystems )
	{
		system->changed( entities );
	}
}

void secs::SystemManager::added(const std::vector<secs::Entity> &entities)
{
	for( auto system : m_entitySystems )
	{
		system->added( entities );
	}
}

void secs::SystemManager::removed(const std::vector<secs::Entity> &entities)
{
	for( auto system : m_entitySystems )
	{
		system->removed( entities );
	}
}

void secs::SystemManager::pushSystem(secs::System::Ptr system)
{
	EntitySystem::Ptr entity_system = dynamic_cast<EntitySystem::Ptr>( system );
	if( nullptr != entity_system )
	{
		entity_system->setComponentFlagsManager( &m_flagsManager );
		m_entitySystems.push_back( entity_system );
	}

	if( system->isRenderingSystem() )
	{
		m_renderingSystems.push_back( system );
	}
	else
	{
		m_systems.push_back( system );
	}

}

void secs::SystemManager::step( float delta )
{
	for( auto system : m_systems )
	{
		system->step( delta );
	}
}

void secs::SystemManager::render()
{
	for( auto system : m_renderingSystems )
	{
		system->step( 0 );
	}
}
