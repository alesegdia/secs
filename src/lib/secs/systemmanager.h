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

	void step( float delta );

	void render();

private:

	/**
	 * @brief m_systems all systems to execute on step()
	 */
	std::vector<System::Ptr>		m_systems;

	/**
	 * @brief m_renderingSystems all rendering systems to execute on render()
	 */
	std::vector<System::Ptr>		m_renderingSystems;

	/**
	 * @brief m_entitySystems all entity systems for entity change/add/removal notification
	 */
	std::vector<EntitySystem::Ptr>	m_entitySystems;

	ComponentFlagsManager& m_flagsManager;

};

}
