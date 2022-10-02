#pragma once

#include <stack>
#include <cstdlib>

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

private:
	std::stack<eid_t> m_recycledEIDs;
	eid_t m_lastEID = 0;

};

}
