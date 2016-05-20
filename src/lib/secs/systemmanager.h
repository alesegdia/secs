#pragma once

#include "entityobserver.h"

namespace secs
{

class SystemManager : public EntityObserver
{
public:

	// EntityObserver interface
	void changed(const std::vector<Entity> &entities)
	{
		for( auto system : m_entitySystems )
		{
			system->changed( entities );
		}
	}

	void added(const std::vector<Entity> &entities)
	{
		for( auto system : m_entitySystems )
		{
			system->added( entities );
		}
	}

	void removed(const std::vector<Entity> &entities)
	{
		for( auto system : m_entitySystems )
		{
			system->removed( entities );
		}
	}

private:
	std::vector<System::Ptr> m_systems;
	std::vector<EntitySystem::Ptr> m_entitySystems;


};

}
