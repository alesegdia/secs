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

	void reset( const Entity& entity );

	void setComponentFlag( const Entity& entity, ctindex_t index );

	void clearComponentFlag( const Entity& entity, ctindex_t index );

	ComponentBits& componentFlags( const Entity& entity );

private:
	std::vector<ComponentBits> m_entityBits;

};

}
