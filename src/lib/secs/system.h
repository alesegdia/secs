#pragma once

namespace secs
{

class System
{
public:

	virtual ~System() = 0 ;

	virtual void step() = 0 ;

};

}
