#include "secs/entityprocessor.h"

namespace secs
{



	ComponentEdit::ComponentEdit(const Entity& entity, Type type, ctindex_t index) : m_entity(entity), m_type(type), m_index(index)
	{

	}

	secs::ComponentEdit::Type ComponentEdit::GetType()
	{
		return m_type;
	}

	secs::ctindex_t ComponentEdit::GetIndex()
	{
		return m_index;
	}

	const secs::Entity& ComponentEdit::GetEntity()
	{
		return m_entity;
	}

	EntityProcessor::EntityProcessor(EntityObserver& system_manager, ComponentManager& component_manager, ComponentFlagsManager& component_flags_manager) : m_systemManager(system_manager),
		m_componentManager(component_manager),
		m_componentFlagsManager(component_flags_manager),
		m_numEntities(0)
	{

	}

	void EntityProcessor::RemoveEntity(const Entity& entity)
	{
		m_numEntities--;
		m_removedEntities.push_back(entity);
	}

	secs::Entity EntityProcessor::AddEntity()
	{
		m_numEntities++;
		Entity entity = m_eidStorage.Retrieve();
		m_addedEntities.push_back(entity);
		m_componentFlagsManager.ResetFlagsForEntity(entity);
		return entity;
	}

	int EntityProcessor::GetNumEntities()
	{
		return m_numEntities;
	}

	void EntityProcessor::ForceApplyChanges()
	{
		ApplyChanges();
	}

	void EntityProcessor::ApplyChanges()
	{
		std::list<Entity> change_list;
		for (ComponentEdit& edit : m_componentEdits)
		{
			change_list.push_back(edit.GetEntity());
			switch (edit.GetType())
			{
			case ComponentEdit::Type::AddComponent:
				m_componentFlagsManager.SetComponentFlagForEntity(edit.GetEntity(), edit.GetIndex());
				break;
			case ComponentEdit::Type::RemoveComponent:
				m_componentFlagsManager.ClearComponentFlagForEntity(edit.GetEntity(), edit.GetIndex());
				break;
			}
		}

		change_list.sort();
		change_list.unique();

		// remove duplicates
		auto& v = m_removedEntities;
		std::sort(v.begin(), v.end());
		auto last = std::unique(v.begin(), v.end());
		v.erase(last, v.end());

		std::vector<Entity> change_vector{ std::begin(change_list), std::end(change_list) };
		std::sort(change_vector.begin(), change_vector.end());
		std::sort(m_addedEntities.begin(), m_addedEntities.end());

		// we discard created entities from the change set because systems will already be notified
		// from m_addedEntities. We could remove this computation from here and let the system manager to
		// perform the difference computation, and pass all 3 sets without filtering at once
		// SystemManager::update( changed, added, removed )
		std::vector<Entity> filtered_change_vector;
		std::set_difference(change_vector.begin(), change_vector.end(),
			m_addedEntities.begin(), m_addedEntities.end(),
			std::back_inserter(filtered_change_vector));

		m_systemManager.OnEntitiesChanged(filtered_change_vector);
		m_systemManager.OnEntitiesAdded(m_addedEntities);
		m_systemManager.OnEntitiesRemoved(m_removedEntities);

		for (auto entity : m_removedEntities)
		{
			m_eidStorage.Recycle(entity);
			m_componentFlagsManager.ClearAllFlagsForEntity(entity);
		}

		m_addedEntities.clear();
		m_removedEntities.clear();
		m_deactivatedEntities.clear();
		m_componentEdits.clear();
	}

}