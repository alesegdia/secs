#pragma once

#include <vector>

#include "entity.h"

namespace secs
{

class EntityObserver
{
public:
	virtual ~EntityObserver() = 0;

	virtual void changed( const std::vector<Entity>& entities ) = 0 ;
	virtual void added( const std::vector<Entity>& entities ) = 0 ;
	virtual void removed( const std::vector<Entity>& entities ) = 0 ;

};

}
