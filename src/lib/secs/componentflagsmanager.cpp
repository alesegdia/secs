
#include <cassert>
#include "componentflagsmanager.h"

void secs::ComponentFlagsManager::ResetFlagsForEntity(const secs::Entity &entity)
{
	if( m_entityBits.size() <= entity.Index() )
	{
		m_entityBits.resize(size_t(entity.Index()) + 1);
	}
	GetComponentFlagsForEntity( entity ).Reset();
}

void secs::ComponentFlagsManager::SetComponentFlagForEntity(const secs::Entity &entity, secs::ctindex_t index)
{
	GetComponentFlagsForEntity( entity ).Set( index, true );
}

void secs::ComponentFlagsManager::ClearComponentFlagForEntity(const secs::Entity &entity, secs::ctindex_t index)
{
	GetComponentFlagsForEntity( entity ).Set( index, false );
}

secs::ComponentBits &secs::ComponentFlagsManager::GetComponentFlagsForEntity(const secs::Entity &entity)
{
	assert( entity.Index() < m_entityBits.size() && "Unknown entity" );
	return m_entityBits[entity.Index()];
}
