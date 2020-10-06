#pragma once

#include "entity.h"
#include "systemmanager.h"
#include "componentmanager.h"
#include "entityprocessor.h"

namespace secs
{

class Engine
{
public:

	typedef Engine* Ptr;

	Engine()
		: m_systemManager( m_componentFlagsManager ),
		  m_entityProcessor( m_systemManager, m_componentManager, m_componentFlagsManager )
	{

    }

    template <typename T, typename... Args>
    std::shared_ptr<T> createSystem(Args&&... args)
    {
        auto sys = std::make_shared<T>(args...);
        pushSystem(sys);
        return sys;
    }

    void pushSystem( System::Ptr system )
	{
        EntitySystem::Ptr entity_system = std::dynamic_pointer_cast<EntitySystem>( system );
        if( nullptr != entity_system )
        {
            entity_system->setComponentFlagsManager( &m_componentFlagsManager );
            entity_system->setEntityProcessor( &m_entityProcessor );
            entity_system->setComponentManager( &m_componentManager );
        }

        m_systemManager.pushSystem( system, entity_system );
	}

    void setSystemGroup( System::Ptr system, SystemGroupIndex group )
    {
        m_systemManager.setSystemGroup( system, group );
    }

    void disableGroup( SystemGroupIndex sgi )
    {
        m_systemManager.disableSystemGroup(sgi);
    }

    void enableGroup( SystemGroupIndex sgi )
    {
        m_systemManager.enableSystemGroup(sgi);
    }

    void activateSystemGroup( SystemGroupIndex sgi)
    {
        m_systemManager.activateSystemGroup(sgi);
    }

	EntityProcessor& processor()
	{
		return m_entityProcessor;
	}

	template <typename ComponentType>
	ComponentType& component( const Entity& e )
	{
		return m_componentManager.componentStorage<ComponentType>()->component(e);
	}

    template <typename ComponentType>
    ComponentType& addComponent(const Entity& e)
    {
        return processor().addComponent<ComponentType>(e);
    }

    void step( double delta )
	{
		m_entityProcessor.applyChanges();
		m_systemManager.step( delta );
	}

	void render()
	{
		m_systemManager.render();
	}

	void deactivate( const Entity e )
	{
        SECS_UNUSED(e);
	}
    
    template <typename T>
    bool hasComponent(const secs::Entity& e)
    {
        return m_componentFlagsManager.hasComponent<T>(e);
    }

private:

	ComponentFlagsManager m_componentFlagsManager;
	SystemManager m_systemManager;
	ComponentManager m_componentManager;
	EntityProcessor m_entityProcessor;

};


}
