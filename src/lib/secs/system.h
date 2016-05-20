#pragma once

namespace secs
{

class System
{
public:

	typedef System* Ptr;

	virtual ~System() = 0 ;

	virtual void step() = 0 ;

};

}
