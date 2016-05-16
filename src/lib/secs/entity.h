#pragma once

#include <cstdint>

namespace secs
{

typedef std::uint32_t eid_t;

class Entity {
public:
	typedef std::uint64_t eid_t;

	Entity( eid_t id )
		: m_id(id)
	{

	}

	eid_t getID() const
	{
		return m_id;
	}

	bool operator==(const Entity& other)
	{
		return m_id == other.getID();
	}

private:
	eid_t m_id;

};

}
