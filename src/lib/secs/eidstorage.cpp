
#include "eidstorage.h"

namespace secs {


eid_t EIDStorage::retrieve()
{
	eid_t eid;
	if( !m_recycledEIDs.empty() )
	{
		eid = m_recycledEIDs.top();
		m_recycledEIDs.pop();
	}
	else
	{
		eid = m_lastEID++;
	}
	m_validEntities.push_back(eid);
	return eid;
}

void EIDStorage::recycle( Entity entity )
{
	auto& v = m_validEntities;
	v.erase(std::remove(v.begin(), v.end(), entity.eid()), v.end());
	m_recycledEIDs.push( secs::eid_t(entity.eid()) );
}

eid_t EIDStorage::lastEID() const
{
	return m_lastEID;
}

size_t EIDStorage::numRecycled() const
{
	return m_recycledEIDs.size();
}

const std::stack<eid_t> &EIDStorage::recycledStack() const
{
	return m_recycledEIDs;
}

}
