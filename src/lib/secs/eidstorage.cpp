
#include "eidstorage.h"

namespace secs {


eid_t EIDStorage::Retrieve()
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

void EIDStorage::Recycle( Entity entity )
{
	auto& v = m_validEntities;
	v.erase(std::remove(v.begin(), v.end(), entity.GetEID()), v.end());
	m_recycledEIDs.push( secs::eid_t(entity.GetEID()) );
}

eid_t EIDStorage::GetLastEID() const
{
	return m_lastEID;
}

size_t EIDStorage::GetNumRecycledEIDs() const
{
	return m_recycledEIDs.size();
}

const std::stack<eid_t> &EIDStorage::GetRecycledStack() const
{
	return m_recycledEIDs;
}

}
