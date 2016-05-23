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

	SystemManager( ComponentFlagsManager& flags_manager );

	// EntityObserver interface
	void changed(const std::vector<Entity> &entities) final;

	void added(const std::vector<Entity> &entities) final;

	void removed(const std::vector<Entity> &entities) final;

	void pushSystem( System::Ptr system );

	void step();

private:
	std::vector<System::Ptr> m_systems;
	ComponentFlagsManager& m_flagsManager;

	// observers?
	std::vector<EntitySystem::Ptr> m_entitySystems;


};

}
