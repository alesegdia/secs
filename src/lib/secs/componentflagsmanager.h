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

	void ResetFlagsForEntity( const Entity& entity );

	void SetComponentFlagForEntity( const Entity& entity, ctindex_t index );

	void ClearComponentFlagForEntity( const Entity& entity, ctindex_t index );

    template <typename ComponentType>
    bool HasComponent( const Entity& entity )
    {
        ctindex_t component_index = ComponentTraits::GetIndexForComponentType<ComponentType>();
        return GetComponentFlagsForEntity(entity)[component_index];
    }

	ComponentBits& GetComponentFlagsForEntity( const Entity& entity );

    void ClearAllFlagsForEntity(const Entity& entity)
    {
		m_entityBits[secs::eid_t(entity.GetEID())].reset();
    }

private:
	std::vector<ComponentBits> m_entityBits;

};

}
