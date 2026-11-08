#include "entity.h"

#include <cassert>

namespace secs {

Entity::Entity(Entity::eid_t id)
    : m_eid(id)
{

}

Entity::Entity(index_t index, generation_t generation)
    : m_eid((static_cast<eid_t>(generation) << 32) | static_cast<eid_t>(index))
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

Entity::index_t Entity::Index() const
{
    assert(m_eid != InvalidEntity);
    return static_cast<index_t>(m_eid & 0xFFFFFFFFull);
}

Entity::generation_t Entity::Generation() const
{
    assert(m_eid != InvalidEntity);
    return static_cast<generation_t>(m_eid >> 32);
}

bool Entity::IsValid() const
{
    return m_eid != InvalidEntity;
}

void Entity::Invalidate()
{
    m_eid = InvalidEntity;
}

bool Entity::operator==(const Entity &other) const
{
    return m_eid == other.m_eid;
}

bool Entity::operator!=(const Entity &other) const
{
    return m_eid != other.m_eid;
}

bool Entity::operator<(const Entity &other) const
{
    return m_eid < other.m_eid;
}

}
