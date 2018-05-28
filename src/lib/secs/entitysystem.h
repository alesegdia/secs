#pragma once

#include <list>
#include <algorithm>
#include <iostream>

#include "entityobserver.h"
#include "system.h"
#include "bits.h"
#include "componenttraits.h"
#include "componentflagsmanager.h"
#include "componentmanager.h"
#include "entityprocessor.h"

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
    virtual void step(double delta ) final ;

	// EntityObserver interface
	void added( const std::vector<Entity> &entities ) final ;
	void removed( const std::vector<Entity> &entities ) final ;
	void changed( const std::vector<Entity> &entities ) final ;

	virtual bool acceptsEntity( const Entity& entity );

	virtual void onAdded( const Entity& e );
    virtual void onRemoved( const Entity& e );
    virtual void process( double delta, const Entity& e ) = 0 ;

	void setComponentFlagsManager( ComponentFlagsManager::Ptr cfm );
    void setEntityProcessor( EntityProcessor::Ptr processor )
    {
        m_entityProcessor = processor;
    }
    void setComponentManager( ComponentManager::Ptr component_manager )
    {
        m_componentManager = component_manager;
    }

protected:
    EntityProcessor::Ptr processor()
    {
        return m_entityProcessor;
    }

	template <typename ComponentType>
	ComponentType& component( const Entity& e )
	{
		return m_componentManager->component<ComponentType>(e);
	}

private:
	void remove( const Entity& entity );
	void add( const Entity& entity );

	ComponentBits m_neededComponents;
	EntityBits m_entityBits;
	std::list<Entity> m_activeEntities;
	ComponentFlagsManager::Ptr m_componentFlagsManager;
	EntityProcessor::Ptr m_entityProcessor;
	ComponentManager::Ptr m_componentManager;

};

/**
 * This class represents an entity system which has automatic component deduction
 */
template <typename... Args>
class TypedEntitySystem : public EntitySystem
{
public:

	typedef TypedEntitySystem<Args...> ParentType;

	TypedEntitySystem()
	{
		setNeededComponents<Args...>();
	}

    void process( double delta, const secs::Entity& e ) final
    {
        process( delta, e, component<Args>(e)... );
    }

    virtual void process(double delta, const secs::Entity& e, Args&... args )
	{

    }

private:
    ComponentManager::Ptr m_componentManager;

};



}
