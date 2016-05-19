#pragma once

#include <stack>
#include <cstdlib>

#include "entity.h"

namespace secs
{

class EIDStorage
{
public:

	eid_t retrieve();
	void recycle( Entity entity );

	// for testing purposes
	eid_t lastEID() const
	{
		return m_lastEID;
	}

	size_t numRecycled() const
	{
		return m_recycledEIDs.size();
	}

	const std::stack<eid_t>& recycledStack() const
	{
		return m_recycledEIDs;
	}

private:
	std::stack<eid_t> m_recycledEIDs;
	eid_t m_lastEID = 0;

};

}
