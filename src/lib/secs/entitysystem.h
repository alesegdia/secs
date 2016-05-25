#pragma once

#include <list>
#include <algorithm>
#include <iostream>

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

	// System interface
	virtual void step( float delta ) final ;

	// EntityObserver interface
	void added( const std::vector<Entity> &entities ) final ;
	void removed( const std::vector<Entity> &entities ) final ;
	void changed( const std::vector<Entity> &entities ) final ;

	virtual bool acceptsEntity( const Entity& entity );

	virtual void onAdded( const Entity& e );
	virtual void onRemoved( const Entity& e );
	virtual void process( float delta, const Entity& e ) = 0 ;

	void setComponentFlagsManager( ComponentFlagsManager::Ptr cfm );


private:

	void remove( const Entity& entity );
	void add( const Entity& entity );

	ComponentBits m_neededComponents;
	EntityBits m_entityBits;
	std::list<Entity> m_activeEntities;
	ComponentFlagsManager::Ptr m_componentFlagsManager;

};

}
