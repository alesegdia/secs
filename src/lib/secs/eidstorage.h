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
	eid_t lastEID() const;

	size_t numRecycled() const;

	const std::stack<eid_t>& recycledStack() const;

private:
	std::stack<eid_t> m_recycledEIDs;
	eid_t m_lastEID = 0;

};

}
