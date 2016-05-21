#pragma once

#include "entityobserver.h"
#include "system.h"
#include "entitysystem.h"
#include "componentflagsmanager.h"

namespace secs
{

class SystemManager : public EntityObserver
{
public:

	SystemManager( ComponentFlagsManager flags_manager )
		: m_flagsManager( flags_manager )
	{

	}

	// EntityObserver interface
	void changed(const std::vector<Entity> &entities) final
	{
		for( auto system : m_entitySystems )
		{
			system->changed( entities );
		}
	}

	void added(const std::vector<Entity> &entities) final
	{
		for( auto system : m_entitySystems )
		{
			system->added( entities );
		}
	}

	void removed(const std::vector<Entity> &entities) final
	{
		for( auto system : m_entitySystems )
		{
			system->removed( entities );
		}
	}

	void pushSystem( System::Ptr system )
	{
		EntitySystem::Ptr entity_system = dynamic_cast<EntitySystem::Ptr>( system );
		if( nullptr != entity_system )
		{
			entity_system->setComponentFlagsManager( &m_flagsManager );
			m_entitySystems.push_back( entity_system );
		}
		m_systems.push_back( system );
	}

	void step()
	{
		for( auto system : m_systems )
		{
			system->step();
		}
	}

private:
	std::vector<System::Ptr> m_systems;
	ComponentFlagsManager& m_flagsManager;

	// observers?
	std::vector<EntitySystem::Ptr> m_entitySystems;


};

}
