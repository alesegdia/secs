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

	eid_t Retrieve();
	void Recycle( Entity entity );

	// for testing purposes
	eid_t GetLastEID() const;

	size_t GetNumRecycledEIDs() const;

	const std::stack<eid_t>& GetRecycledStack() const;

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
