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

    template <typename ComponentType>
    bool hasComponent( const Entity& entity )
    {
        ctindex_t component_index = ComponentTraits::getIndex<ComponentType>();
        return componentFlags(entity)[component_index];
    }

	ComponentBits& componentFlags( const Entity& entity );

    void clearAllFlags(const Entity& entity)
    {
		m_entityBits[secs::eid_t(entity.eid())].reset();
    }

private:
	std::vector<ComponentBits> m_entityBits;

};

}
