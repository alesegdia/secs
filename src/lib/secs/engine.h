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

	Engine()
		: m_systemManager( m_componentFlagsManager ),
		  m_entityProcessor( m_systemManager, m_componentManager, m_componentFlagsManager )
	{

	}

	void pushSystem( System::Ptr system )
	{
		m_systemManager.pushSystem( system );
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

	void step( float delta )
	{
		m_entityProcessor.applyChanges();
		m_systemManager.step( delta );
	}

	void render()
	{
		m_systemManager.render();
	}

private:

	ComponentFlagsManager m_componentFlagsManager;
	SystemManager m_systemManager;
	ComponentManager m_componentManager;
	EntityProcessor m_entityProcessor;

};


}
