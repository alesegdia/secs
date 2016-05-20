#pragma once

#include "entityobserver.h"
#include "system.h"
#include "entitysystem.h"

namespace secs
{

class SystemManager : public EntityObserver
{
public:

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
		if( dynamic_cast<EntitySystem::Ptr>( system ) != nullptr )
		{
			m_entitySystems.push_back( static_cast<EntitySystem::Ptr>(system) );
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

	// observers?
	std::vector<EntitySystem::Ptr> m_entitySystems;


};

}
