#pragma once

#include "entity.h"
#include "eidstorage.h"

namespace secs
{

class EntityManager
{
public:
	EntityManager()
	{

	}

private:
	EIDStorage m_eidStorage;

};

}
