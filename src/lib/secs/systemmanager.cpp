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

void secs::SystemManager::changed(const std::vector<secs::Entity> &entities)
{
    for( auto system : m_entitySystems )
    {
        system->changed( entities );
    }
}

void secs::SystemManager::added(const std::vector<secs::Entity> &entities)
{
    for( auto system : m_entitySystems )
	{
		system->added( entities );
	}
}

void secs::SystemManager::removed(const std::vector<secs::Entity> &entities)
{
	for( auto system : m_entitySystems )
	{
		system->removed( entities );
	}
}

void secs::SystemManager::pushSystem( secs::System::Ptr system, EntitySystem::Ptr entity_system )
{
    if( entity_system != nullptr)
    {
        m_entitySystems.push_back( entity_system );
	}

    if( system->hasRenderingStep() )
	{
        insertSorted( system, m_renderingSystems );
    }

    if( system->hasProcessingStep())
    {
        insertSorted( system, m_processingSystems );
    }

}

void secs::SystemManager::activateSystemGroup(secs::SystemGroupIndex sgi)
{
    assert( sgi < MaxSystemGroups );
    assert( m_systemsByGroup[sgi] == nullptr && "System group already activated!" );
    m_systemsByGroup[sgi] = new std::vector<System::Ptr>();
}

void secs::SystemManager::setSystemGroup(secs::System::Ptr system, secs::SystemGroupIndex group)
{
    assert( m_systemsByGroup != nullptr && "System group not activated! Use activateSystemGroup.");
    assert( group < MaxSystemGroups );
    m_systemsByGroup[group]->push_back(system);
}

void secs::SystemManager::disableSystemGroup(secs::SystemGroupIndex sgi)
{
    assert( m_systemsByGroup[sgi] != nullptr );
    for( System::Ptr s : *(m_systemsByGroup[sgi]) )
    {
        s->disable();
    }
}

void secs::SystemManager::enableSystemGroup(secs::SystemGroupIndex sgi)
{
    assert( m_systemsByGroup[sgi] != nullptr );
    for( System::Ptr s : *(m_systemsByGroup[sgi]) )
    {
        s->enable();
    }
}

void secs::SystemManager::step( double delta )
{
    for( System::Ptr system : m_processingSystems )
    {
        system->preUpdate( delta );
        if( system->isEnabled() )
        {
            system->step( delta );
        }
        system->postUpdate( delta );
    }
}

void secs::SystemManager::render()
{
    for( System::Ptr system : m_renderingSystems )
    {
        if( system->isEnabled() )
        {
            system->renderStep( );
        }
    }
}

void secs::SystemManager::insertSorted(secs::System::Ptr system, std::vector<secs::System::Ptr>& vec)
{
    auto comparator = []( System::Ptr s1, System::Ptr s2 )
    {
        return s1->executionPriority() < s2->executionPriority();
    };
    auto it = std::upper_bound( vec.begin(), vec.end(), system, comparator);
    vec.insert(it, system);
}
