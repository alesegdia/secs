#pragma once

#include <vector>

#include "entity.h"

namespace secs
{

class EntityObserver
{
public:

	typedef EntityObserver* Ptr;

	virtual ~EntityObserver() = 0;

	virtual void OnEntitiesChanged( const std::vector<Entity>& entities ) = 0 ;
	virtual void OnEntitiesAdded( const std::vector<Entity>& entities ) = 0 ;
	virtual void OnEntitiesRemoved( const std::vector<Entity>& entities ) = 0 ;

};

}
