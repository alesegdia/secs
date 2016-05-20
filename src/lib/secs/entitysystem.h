#pragma once

#include <list>
#include <algorithm>

#include "entityobserver.h"
#include "system.h"
#include "bits.h"

namespace secs
{

class EntitySystem : public System, public EntityObserver
{
public:

	typedef EntitySystem* Ptr;

	~EntitySystem() = 0;

	// EntityObserver interface
	void added( const std::vector<Entity> &entities ) final;

	void removed( const std::vector<Entity> &entities ) final;

	void changed(const std::vector<Entity> &entities) final;

	virtual bool acceptsEntity( const Entity& e ) = 0 ;

	virtual void onAdded( const Entity& e ) = 0 ;
	virtual void onRemoved( const Entity& e ) = 0 ;
	virtual void process( const Entity& e ) = 0 ;

	// System interface
	virtual void step() final;

private:

	void remove( const Entity& entity );

	void add( const Entity& entity );

	EntityBits m_entityBits;
	std::list<Entity> m_activeEntities;

};

}
