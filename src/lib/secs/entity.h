#pragma once

#include <cstdint>

namespace secs
{

typedef std::uint32_t eid_t;

class Entity {
public:
	typedef std::uint64_t eid_t;

	Entity( eid_t id );

	eid_t eid() const;

	bool operator==( const Entity& other ) ;
	bool operator<( const Entity& other ) const;

private:
	eid_t m_eid;

};

}
