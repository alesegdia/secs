#include "systemmanager.h"

secs::SystemManager::SystemManager(secs::ComponentFlagsManager &flags_manager)
	: m_flagsManager( flags_manager )
{
    for( secs::SystemGroupIndex i = 0; i < MaxSystemGroups; i++ )
    {
        m_systemsByGroup[i] = nullptr;
    }
}

secs::SystemManager::~SystemManager()
{
    for( size_t i = 0; i < MaxSystemGroups; i++ )
    {
        if( m_systemsByGroup[i] != nullptr )
        {
            delete m_systemsByGroup[i];
        }
    }
}

void secs::SystemManager::OnEntitiesChanged(const std::vector<secs::Entity> &entities)
{
    for( auto system : m_entitySystems )
    {
        system->OnEntitiesChanged( entities );
    }
}

void secs::SystemManager::OnEntitiesAdded(const std::vector<secs::Entity> &entities)
{
    for( auto system : m_entitySystems )
	{
		system->OnEntitiesAdded( entities );
	}
}

void secs::SystemManager::OnEntitiesRemoved(const std::vector<secs::Entity> &entities)
{
	for( auto system : m_entitySystems )
	{
		system->OnEntitiesRemoved( entities );
	}
}

void secs::SystemManager::PushSystem( secs::System::Ptr system, EntitySystem::Ptr entity_system )
{
    if( entity_system != nullptr)
    {
        m_entitySystems.push_back( entity_system );
	}

    if( system->HasRenderingStep() )
	{
        insertSorted( system, m_renderingSystems );
    }

    if( system->HasProcessingStep())
    {
        insertSorted( system, m_processingSystems );
    }

}

void secs::SystemManager::ActivateSystemGroup(secs::SystemGroupIndex sgi)
{
    assert( sgi < MaxSystemGroups );
    assert( m_systemsByGroup[sgi] == nullptr && "System group already activated!" );
    m_systemsByGroup[sgi] = new std::vector<System::Ptr>();
}

void secs::SystemManager::SetSystemGroup(secs::System::Ptr system, secs::SystemGroupIndex group)
{
    assert( m_systemsByGroup != nullptr && "System group not activated! Use activateSystemGroup.");
    assert( group < MaxSystemGroups );
    m_systemsByGroup[group]->push_back(system);
}

void secs::SystemManager::DisableSystemGroup(secs::SystemGroupIndex sgi)
{
    assert( m_systemsByGroup[sgi] != nullptr );
    for( System::Ptr s : *(m_systemsByGroup[sgi]) )
    {
        s->Disable();
    }
}

void secs::SystemManager::EnableSystemGroup(secs::SystemGroupIndex sgi)
{
    assert( m_systemsByGroup[sgi] != nullptr );
    for( System::Ptr s : *(m_systemsByGroup[sgi]) )
    {
        s->Enable();
    }
}

void secs::SystemManager::Step( double delta )
{
    // std::cout << "start update systems" << std::endl;
    for( System::Ptr system : m_processingSystems )
    {
        system->PreUpdate( delta );
        if( system->IsEnabled() )
        {
            system->Step( delta );
        }
        system->PostUpdate( delta );
    }
    // std::cout << "end update systems" << std::endl;
}

void secs::SystemManager::Render()
{
    for( System::Ptr system : m_renderingSystems )
    {
        if( system->IsEnabled() )
        {
            system->RenderStep( );
        }
    }
}

void secs::SystemManager::insertSorted(secs::System::Ptr system, std::vector<secs::System::Ptr>& vec)
{
    auto comparator = []( System::Ptr s1, System::Ptr s2 )
    {
        return s1->GetExecutionPriority() < s2->GetExecutionPriority();
    };
    auto it = std::upper_bound( vec.begin(), vec.end(), system, comparator);
    vec.insert(it, system);
}
