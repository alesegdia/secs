#pragma once

namespace secs
{

class System
{
public:

	typedef System* Ptr;

	virtual ~System();

	virtual void step() = 0 ;

};

}
