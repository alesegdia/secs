
#include <iostream>
#include "componentflagsmanager.h"

void secs::ComponentFlagsManager::reset(const secs::Entity &entity)
{
	if( m_entityBits.size() < ( entity.eid() + 1 ) )
	{
		m_entityBits.resize(secs::eid_t(entity.eid() + 1));
	}
	componentFlags( entity ).reset();
}

void secs::ComponentFlagsManager::setComponentFlag(const secs::Entity &entity, secs::ctindex_t index)
{
	componentFlags( entity ).set( index, true );
}

void secs::ComponentFlagsManager::clearComponentFlag(const secs::Entity &entity, secs::ctindex_t index)
{
	componentFlags( entity ).set( index, false );
}

secs::ComponentBits &secs::ComponentFlagsManager::componentFlags(const secs::Entity &entity)
{
	return m_entityBits[secs::eid_t(entity.eid())];
}
