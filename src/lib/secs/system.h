#pragma once

namespace secs
{

class System
{
public:

	typedef System* Ptr;

	virtual ~System();

	virtual void step( float delta ) = 0 ;

};

}
