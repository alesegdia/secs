#pragma once

#include <stack>
#include <cstdlib>
#include <vector>

#include "entity.h"

namespace secs
{

class EIDStorage
{
public:

	eid_t retrieve();
	void recycle( Entity entity );

	// for testing purposes
	eid_t lastEID() const;

	size_t numRecycled() const;

	const std::stack<eid_t>& recycledStack() const;

	std::vector<eid_t> validEntities()
	{
		return m_validEntities;
	}

private:
	std::stack<eid_t> m_recycledEIDs;
	std::vector<eid_t> m_validEntities;
	eid_t m_lastEID = 0;

};

}
