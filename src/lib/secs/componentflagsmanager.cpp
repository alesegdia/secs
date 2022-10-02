
#include <iostream>
#include "componentflagsmanager.h"

void secs::ComponentFlagsManager::ResetFlagsForEntity(const secs::Entity &entity)
{
	if( m_entityBits.size() < ( entity.GetEID() + 1 ) )
	{
		m_entityBits.resize(secs::eid_t(entity.GetEID() + 1));
	}
	GetComponentFlagsForEntity( entity ).reset();
}

void secs::ComponentFlagsManager::SetComponentFlagForEntity(const secs::Entity &entity, secs::ctindex_t index)
{
	GetComponentFlagsForEntity( entity ).set( index, true );
}

void secs::ComponentFlagsManager::ClearComponentFlagForEntity(const secs::Entity &entity, secs::ctindex_t index)
{
	GetComponentFlagsForEntity( entity ).set( index, false );
}

secs::ComponentBits &secs::ComponentFlagsManager::GetComponentFlagsForEntity(const secs::Entity &entity)
{
	return m_entityBits[secs::eid_t(entity.GetEID())];
}
