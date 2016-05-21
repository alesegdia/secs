#pragma once

#include "componenttraits.h"
#include "bits.h"
#include "entity.h"

#include <vector>

namespace secs
{

class ComponentFlagsManager
{
public:

	typedef ComponentFlagsManager* Ptr;

	void reset( const Entity& entity )
	{
		if( m_entityBits.size() < ( entity.eid() + 1 ) )
		{
			m_entityBits.resize(entity.eid() + 1);
		}
		m_entityBits[entity.eid()].reset();
	}

	void setComponentFlag( const Entity& entity, ctindex_t index )
	{
		componentFlags( entity.eid() ).set( index, true );
	}

	void clearComponentFlag( const Entity& entity, ctindex_t index )
	{
		componentFlags( entity.eid() ).set( index, false );
	}

	ComponentBits& componentFlags( const Entity& entity )
	{
		return m_entityBits[entity.eid()];
	}

private:
	std::vector<ComponentBits> m_entityBits;

};

}
