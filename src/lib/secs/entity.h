#pragma once

#include <cstdint>
#include <limits>

namespace secs
{

typedef uint32_t eid_t;

class Entity {
public:
    using eid_t = uint64_t;
    static constexpr eid_t InvalidEntity = std::numeric_limits<eid_t>::max();

	Entity( eid_t id );
    Entity();

	eid_t eid() const;

    bool isValid() const;

    void invalidate();

	bool operator==( const Entity& other ) ;
	bool operator<( const Entity& other ) const;

private:
	eid_t m_eid;

};

}
