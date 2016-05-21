#pragma once

#include <list>
#include <algorithm>

#include "entityobserver.h"
#include "system.h"
#include "bits.h"
#include "componenttraits.h"
#include "componentflagsmanager.h"

namespace secs
{

class EntitySystem : public System, public EntityObserver
{
public:

	typedef EntitySystem* Ptr;

	template <typename... Args>
	void setNeededComponents ()
	{
		m_neededComponents = ComponentBitsBuilder<Args...>::buildBits();
	}

	~EntitySystem() = 0;

	// EntityObserver interface
	void added( const std::vector<Entity> &entities ) final;
	void removed( const std::vector<Entity> &entities ) final;
	void changed( const std::vector<Entity> &entities ) final;

	virtual bool acceptsEntity( ComponentBits entity_component_bits )
	{
		return ( m_neededComponents & entity_component_bits ) == m_neededComponents;
	}

	virtual void onAdded( const Entity& e ) = 0 ;
	virtual void onRemoved( const Entity& e ) = 0 ;
	virtual void process( const Entity& e ) = 0 ;

	void setComponentFlagsManager( ComponentFlagsManager::Ptr cfm )
	{
		m_componentFlagsManager = cfm;
	}

	// System interface
	virtual void step() final;

private:

	void remove( const Entity& entity );
	void add( const Entity& entity );

	ComponentBits m_neededComponents;
	EntityBits m_entityBits;
	std::list<Entity> m_activeEntities;
	ComponentFlagsManager::Ptr m_componentFlagsManager;

};

}
