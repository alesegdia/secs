#pragma once

#include <cstdint>

namespace secs
{

typedef std::uint32_t eid_t;

class Entity {
public:
	typedef std::uint64_t eid_t;

	Entity( eid_t id )
		: m_eid(id)
	{

	}

	eid_t eid() const
	{
		return m_eid;
	}

	bool operator==(const Entity& other)
	{
		return m_eid == other.eid();
	}

private:
	eid_t m_eid;

};

}
