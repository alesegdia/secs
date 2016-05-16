#pragma once

#include <stack>

#include "entity.h"

namespace secs
{

class EIDStorage
{
public:
	eid_t retrieve();

	void recycle( Entity entity );

private:
	std::stack<eid_t> m_recycledEIDs;
	eid_t m_lastEID = 0;

};

}
