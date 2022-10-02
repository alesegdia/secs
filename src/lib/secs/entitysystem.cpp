
#include "entitysystem.h"

secs::EntitySystem::~EntitySystem()
= default;

void secs::EntitySystem::OnEntitiesAdded(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		if( AcceptsEntity( entity ) )
		{
			AddEntity( entity );
		}
	}
}

void secs::EntitySystem::OnEntitiesRemoved(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		if( m_entityBits[secs::eid_t(entity.GetEID())] )
		{
			RemoveEntity( entity );
		}
	}
}

void secs::EntitySystem::OnEntitiesChanged(const std::vector<secs::Entity> &entities)
{
	for( const Entity& entity : entities )
	{
		bool already_present = m_entityBits.test(secs::eid_t(entity.GetEID()) );
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
	ComponentBits &entity_component_bits = m_componentFlagsManager->GetComponentFlagsForEntity( entity );
	return ( m_neededComponents & entity_component_bits ) == m_neededComponents;
}

void secs::EntitySystem::OnEntityAdded(const secs::Entity &e)
{
    SECS_UNUSED(e);
}

void secs::EntitySystem::OnEntityRemoved(const secs::Entity &e)
{
    SECS_UNUSED(e);
}

void secs::EntitySystem::Process(double delta, const Entity& e)
{
	SECS_UNUSED(delta);
	SECS_UNUSED(e);
}

void secs::EntitySystem::Render(const Entity& e)
{
	SECS_UNUSED(e);
}

void secs::EntitySystem::SetComponentFlagsManager(secs::ComponentFlagsManager::Ptr cfm)
{
	m_componentFlagsManager = cfm;
}

void secs::EntitySystem::Step(double delta)
{
    for( const Entity& e : m_activeEntities )
    {
        Process( delta, e );
    }
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
	size_t pos = size_t(entity.GetEID());
	m_entityBits.set( pos, false );
	m_activeEntities.erase( std::remove( m_activeEntities.begin(), m_activeEntities.end(), entity ), m_activeEntities.end() );
    OnEntityRemoved( entity );fflush(0);
}

void secs::EntitySystem::AddEntity(const secs::Entity &entity)
{
	size_t pos = size_t(entity.GetEID());
    m_entityBits.set( pos, true );
    m_activeEntities.push_back( entity );
	OnEntityAdded( entity );
}


