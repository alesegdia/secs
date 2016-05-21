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

	void step()
	{
		m_entityProcessor.applyChanges();
		m_systemManager.step();
	}

private:

	ComponentFlagsManager m_componentFlagsManager;
	SystemManager m_systemManager;
	ComponentManager m_componentManager;
	EntityProcessor m_entityProcessor;

};


}
