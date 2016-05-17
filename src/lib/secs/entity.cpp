#include "entity.h"

namespace secs {

Entity::Entity(Entity::eid_t id)
	: m_eid(id)
{

}

Entity::eid_t Entity::eid() const
{
	return m_eid;
}

bool Entity::operator==(const Entity &other)
{
	return m_eid == other.eid();
}



}
