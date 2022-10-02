#pragma once

#include <algorithm>
#include <functional>

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

    using EntityVisitor = std::function<void(const secs::Entity& e)>;

	Engine()
		: m_systemManager( m_componentFlagsManager ),
		  m_entityProcessor( m_systemManager, m_componentManager, m_componentFlagsManager )
	{

    }

    void Visit(EntityVisitor v)
    {
        for (auto e : GetEntityProcessor().ValidEntities())
        {
            v(Entity(e));
        }
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> CreateSystem(Args&&... args)
    {
        auto sys = std::make_shared<T>(args...);
        PushSystem(sys);
        return sys;
    }

    void PushSystem( System::Ptr system )
	{
        EntitySystem::Ptr entity_system = std::dynamic_pointer_cast<EntitySystem>( system );
        if( nullptr != entity_system )
        {
            entity_system->SetComponentFlagsManager( &m_componentFlagsManager );
            entity_system->SetEntityProcessor( &m_entityProcessor );
            entity_system->SetComponentManager( &m_componentManager );
        }

        m_systemManager.PushSystem( system, entity_system );
	}

    void SetSystemGroup( System::Ptr system, SystemGroupIndex group )
    {
        m_systemManager.SetSystemGroup( system, group );
    }

    void DisableGroup( SystemGroupIndex sgi )
    {
        m_systemManager.DisableSystemGroup(sgi);
    }

    void EnableGroup( SystemGroupIndex sgi )
    {
        m_systemManager.EnableSystemGroup(sgi);
    }

    void ActivateSystemGroup( SystemGroupIndex sgi)
    {
        m_systemManager.ActivateSystemGroup(sgi);
    }

	EntityProcessor& GetEntityProcessor()
	{
		return m_entityProcessor;
	}

	template <typename ComponentType>
	ComponentType& GetComponent( const Entity& e )
	{
		return m_componentManager.GetComponentStorageForComponentType<ComponentType>()->GetComponent(e);
	}

    template <typename ComponentType>
    ComponentType& AddComponent(const Entity& e)
    {
        return GetEntityProcessor().AddComponent<ComponentType>(e);
    }

    void Step( double delta )
	{
		m_entityProcessor.ApplyChanges();
		m_systemManager.Step( delta );
	}

	void Render()
	{
		m_systemManager.Render();
	}

	void Deactivate( const Entity e )
	{
        SECS_UNUSED(e);
	}
    
    template <typename T>
    bool HasComponent(const secs::Entity& e)
    {
        return m_componentFlagsManager.HasComponent<T>(e);
    }

private:

	ComponentFlagsManager m_componentFlagsManager;
	SystemManager m_systemManager;
	ComponentManager m_componentManager;
	EntityProcessor m_entityProcessor;

};


}
