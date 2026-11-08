
#include "eidstorage.h"

#include <algorithm>

namespace secs {


Entity EIDStorage::Retrieve()
{
	Entity::index_t index;
	if( !m_recycledIndices.empty() )
	{
		index = m_recycledIndices.back();
		m_recycledIndices.pop_back();
	}
	else
	{
		index = static_cast<Entity::index_t>(m_generations.size());
		m_generations.push_back(0);
		m_alive.push_back(false);
	}
	m_alive[index] = true;
	Entity entity(index, m_generations[index]);
	m_validEntities.push_back(entity);
	return entity;
}

void EIDStorage::Recycle( const Entity& entity )
{
	if( !IsAlive(entity) )
	{
		return;
	}
	auto& v = m_validEntities;
	v.erase(std::remove(v.begin(), v.end(), entity), v.end());

	const auto index = entity.Index();
	m_alive[index] = false;
	m_generations[index]++;
	m_recycledIndices.push_back(index);
}

bool EIDStorage::IsAlive(const Entity& entity) const
{
	if( !entity.IsValid() )
	{
		return false;
	}
	const auto index = entity.Index();
	return index < m_generations.size()
		&& m_alive[index]
		&& m_generations[index] == entity.Generation();
}

eid_t EIDStorage::GetLastEID() const
{
	return static_cast<eid_t>(m_generations.size());
}

size_t EIDStorage::GetNumRecycledEIDs() const
{
	return m_recycledIndices.size();
}

}
