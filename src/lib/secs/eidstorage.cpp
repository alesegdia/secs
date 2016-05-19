
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
	return eid;
}

void EIDStorage::recycle( Entity entity )
{
	m_recycledEIDs.push( entity.eid() );
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
