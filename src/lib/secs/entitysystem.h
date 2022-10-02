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

    typedef std::shared_ptr<EntitySystem> Ptr;

	template <typename... Args>
	void SetNeededComponents ()
	{
		m_neededComponents = ComponentBitsBuilder<Args...>::BuildBits();
	}

	template <typename... Args>
	void AddNeededComponents ()
	{
		m_neededComponents |= ComponentBitsBuilder<Args...>::BuildBits();
	}

	~EntitySystem() = 0;

	// System interface
    void Step(double delta) final ;
    void RenderStep() override ;

	// EntityObserver interface
	void OnEntitiesAdded( const std::vector<Entity> &entities ) final ;
	void OnEntitiesRemoved( const std::vector<Entity> &entities ) final ;
	void OnEntitiesChanged( const std::vector<Entity> &entities ) final ;

	virtual bool AcceptsEntity( const Entity& entity );

	virtual void OnEntityAdded( const Entity& e );
    virtual void OnEntityRemoved( const Entity& e );

    virtual void Process( double delta, const Entity& e );

    virtual void Render( const Entity& e );

	void SetComponentFlagsManager( ComponentFlagsManager::Ptr cfm );
    void SetEntityProcessor( EntityProcessor::Ptr processor )
    {
        m_entityProcessor = processor;
    }
    void SetComponentManager( ComponentManager::Ptr component_manager ) {
        m_componentManager = component_manager;
    }

    template <typename ComponentType>
    ComponentType& AddComponent(const secs::Entity& e)
    {
        return m_entityProcessor->AddComponent<ComponentType>(e);
    }

protected:
    EntityProcessor::Ptr GetEntityProcessor();

	template <typename ComponentType>
	ComponentType& GetComponent( const Entity& e )
	{
		return m_componentManager->GetComponentForEntity<ComponentType>(e);
	}

    template <typename ComponentType>
    bool HasComponent( const Entity& e )
    {
        return m_componentFlagsManager->HasComponent<ComponentType>(e);
    }

private:
	void RemoveEntity( const Entity& entity );
	void AddEntity( const Entity& entity );

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
		SetNeededComponents<Args...>();
	}

    virtual ~TypedEntitySystem()
    {

    }

    void Process( double delta, const secs::Entity& e ) final
    {
        process( delta, e, GetComponent<Args>(e)... );
    }

    virtual void process(double delta, const secs::Entity& e, Args&... args ) = 0;

private:
    ComponentManager::Ptr m_componentManager;

};



}
