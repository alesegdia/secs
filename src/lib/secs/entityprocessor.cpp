#include "entityprocessor.h"

#include <iterator>
#include <utility>

namespace secs
{

	ComponentEdit::ComponentEdit(const Entity& entity, Type type, ctindex_t index) : m_entity(entity), m_type(type), m_index(index)
	{

	}

	secs::ComponentEdit::Type ComponentEdit::GetType() const
	{
		return m_type;
	}

	secs::ctindex_t ComponentEdit::GetIndex() const
	{
		return m_index;
	}

	const secs::Entity& ComponentEdit::GetEntity() const
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
		if (!IsAlive(entity) || IsPendingRemoval(entity))
		{
			return;
		}
		if (entity.Index() >= m_pendingRemoval.size())
		{
			m_pendingRemoval.resize(size_t(entity.Index()) + 1, false);
		}
		m_pendingRemoval[entity.Index()] = true;
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

	int EntityProcessor::GetNumEntities() const
	{
		return m_numEntities;
	}

	void EntityProcessor::ForceApplyChanges()
	{
		ApplyChanges();
	}

	void EntityProcessor::RemoveFromTags(const Entity& entity)
	{
		for (auto& [tag, entities] : m_taggedEntities)
		{
			entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
		}
	}

	void EntityProcessor::ApplyChanges()
	{
		int passes = 0;
		while (!m_componentEdits.empty() || !m_addedEntities.empty() || !m_removedEntities.empty())
		{
			passes++;
			assert(passes <= MaxApplyPasses && "System callbacks keep producing entity changes");
			if (passes > MaxApplyPasses)
			{
				break;
			}

			// Take ownership of this pass's changes. Callbacks below may queue new
			// changes; those land in the (now empty) member buffers and are handled
			// by the next pass.
			auto edits   = std::exchange(m_componentEdits, {});
			auto added   = std::exchange(m_addedEntities, {});
			auto removed = std::exchange(m_removedEntities, {});

			std::vector<Entity> changed;
			changed.reserve(edits.size());
			for (const ComponentEdit& edit : edits)
			{
				changed.push_back(edit.GetEntity());
			}
			std::sort(changed.begin(), changed.end());
			changed.erase(std::unique(changed.begin(), changed.end()), changed.end());
			std::sort(added.begin(), added.end());

			// Created entities are announced through OnEntitiesAdded, so they are
			// left out of the changed set.
			std::vector<Entity> filtered_changed;
			std::set_difference(changed.begin(), changed.end(),
				added.begin(), added.end(),
				std::back_inserter(filtered_changed));

			m_systemManager.OnEntitiesChanged(filtered_changed);
			m_systemManager.OnEntitiesAdded(added);
			m_systemManager.OnEntitiesRemoved(removed);

			// Systems have been notified: now free removed components, unless the
			// component was added back in the meantime.
			for (const ComponentEdit& edit : edits)
			{
				if (edit.GetType() == ComponentEdit::Type::RemoveComponent
					&& IsAlive(edit.GetEntity())
					&& !m_componentFlagsManager.HasComponent(edit.GetEntity(), edit.GetIndex()))
				{
					m_componentManager.FreeComponent(edit.GetEntity(), edit.GetIndex());
				}
			}

			for (const Entity& entity : removed)
			{
				m_componentManager.FreeAllComponents(entity);
				m_componentFlagsManager.ClearAllFlagsForEntity(entity);
				RemoveFromTags(entity);
				m_pendingRemoval[entity.Index()] = false;
				m_eidStorage.Recycle(entity);
			}
		}

		m_deactivatedEntities.clear();
	}

}
