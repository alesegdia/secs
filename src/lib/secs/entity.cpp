#include "entity.h"

#include <cassert>

namespace secs {

Entity::Entity(Entity::eid_t id)
    : m_eid(id)
{

}

Entity::Entity()
    : Entity(InvalidEntity)
{

}

Entity::eid_t Entity::eid() const
{
    assert(m_eid != InvalidEntity);
	return m_eid;
}

bool Entity::isValid()
{
    return m_eid != InvalidEntity;
}

void Entity::invalidate()
{
    m_eid = InvalidEntity;
}

bool Entity::operator==(const Entity &other)
{
    return m_eid == other.eid();
}

bool Entity::operator<(const Entity &other) const
{
	return m_eid < other.m_eid;
}



}
