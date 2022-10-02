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

Entity::eid_t Entity::GetEID() const
{
    assert(m_eid != InvalidEntity);
	return m_eid;
}

bool Entity::IsValid() const
{
    return m_eid != InvalidEntity;
}

void Entity::Invalidate()
{
    m_eid = InvalidEntity;
}

bool Entity::operator==(const Entity &other)
{
    return m_eid == other.GetEID();
}

bool Entity::operator<(const Entity &other) const
{
	return m_eid < other.m_eid;
}



}
