
#include "entitysystem.h"

secs::EntitySystem::~EntitySystem()
= default;

void secs::EntitySystem::OnEntitiesAdded(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		// Idempotent: an entity may already be present if the system was backfilled
		// with existing entities when it was pushed (see Engine::PushSystem).
		if( !m_entityBits.Test( entity.Index() ) && AcceptsEntity( entity ) )
		{
			AddEntity( entity );
		}
	}
}

void secs::EntitySystem::OnEntitiesRemoved(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		if( m_entityBits.Test( entity.Index() ) )
		{
			RemoveEntity( entity );
		}
	}
}

void secs::EntitySystem::OnEntitiesChanged(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		bool already_present = m_entityBits.Test( entity.Index() );
		bool system_accept = AcceptsEntity( entity );
		if( already_present && !system_accept )
		{
			RemoveEntity( entity );
		}
		else if( !already_present && system_accept )
		{
			AddEntity( entity );
		}
	}
}

bool secs::EntitySystem::AcceptsEntity(const secs::Entity& entity)
{
	const ComponentBits &entity_component_bits = m_componentFlagsManager->GetComponentFlagsForEntity( entity );
	return m_neededComponents.IsSubsetOf( entity_component_bits );
}

void secs::EntitySystem::OnEntityAdded(const secs::Entity &)
{

}

void secs::EntitySystem::OnEntityRemoved(const secs::Entity &)
{

}

void secs::EntitySystem::Process(double, const Entity&)
{

}

void secs::EntitySystem::Render(const Entity&)
{
}

void secs::EntitySystem::SetComponentFlagsManager(secs::ComponentFlagsManager::Ptr cfm)
{
	m_componentFlagsManager = cfm;
}

void secs::EntitySystem::Step(double delta)
{
	// std::cout << "step sys" << std::endl;
    for( const Entity& e : m_activeEntities )
    {
        Process( delta, e );
    }
	// std::cout << "step sys end" << std::endl;
}

void secs::EntitySystem::RenderStep()
{
    for( const Entity& e : m_activeEntities )
    {
        Render( e );
    }
}

secs::EntityProcessor::Ptr secs::EntitySystem::GetEntityProcessor()
{
	return m_entityProcessor;
}

void secs::EntitySystem::RemoveEntity(const secs::Entity &entity)
{
	size_t pos = size_t(entity.Index());
	m_entityBits.Set( pos, false );
	m_activeEntities.erase( std::remove( m_activeEntities.begin(), m_activeEntities.end(), entity ), m_activeEntities.end() );
    OnEntityRemoved( entity );fflush(0);
}

void secs::EntitySystem::AddEntity(const secs::Entity &entity)
{
	size_t pos = size_t(entity.Index());
    m_entityBits.Set( pos, true );
    m_activeEntities.push_back( entity );
	OnEntityAdded( entity );
}


