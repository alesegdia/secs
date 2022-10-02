#pragma once

#include "entityobserver.h"
#include "system.h"
#include "entitysystem.h"
#include "componentflagsmanager.h"

#include <unordered_map>

namespace secs
{

typedef size_t SystemGroupIndex;

class SystemManager : public EntityObserver
{
public:

    static constexpr SystemGroupIndex MaxSystemGroups = 32;

    SystemManager( ComponentFlagsManager& flags_manager );

    ~SystemManager();

	// EntityObserver interface
	void OnEntitiesChanged(const std::vector<Entity> &entities) final;

	void OnEntitiesAdded(const std::vector<Entity> &entities) final;

	void OnEntitiesRemoved(const std::vector<Entity> &entities) final;

    void PushSystem( secs::System::Ptr system, EntitySystem::Ptr entity_system );

    void ActivateSystemGroup( SystemGroupIndex sgi );

    void SetSystemGroup( System::Ptr system, SystemGroupIndex group );

    void DisableSystemGroup( SystemGroupIndex sgi );

    void EnableSystemGroup( SystemGroupIndex sgi );

    void Step(double delta );

	void Render();

private:

    void insertSorted(System::Ptr system, std::vector<System::Ptr> &vec );

	/**
	 * @brief m_systems all systems to execute on step()
	 */
    std::vector<System::Ptr>		m_processingSystems;

	/**
	 * @brief m_renderingSystems all rendering systems to execute on render()
	 */
    std::vector<System::Ptr>		m_renderingSystems;

	/**
	 * @brief m_entitySystems all entity systems for entity change/add/removal notification
	 */
	std::vector<EntitySystem::Ptr>	m_entitySystems;

	ComponentFlagsManager& m_flagsManager;

    std::vector<System::Ptr>* m_systemsByGroup[MaxSystemGroups]{};

};

}
