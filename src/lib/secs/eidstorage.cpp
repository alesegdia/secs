
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

}
