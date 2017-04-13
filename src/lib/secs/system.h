#pragma once

namespace secs
{

class System
{
public:

	typedef System* Ptr;

	virtual ~System();

    virtual void step( double delta ) = 0 ;

	bool isRenderingSystem()
	{
		return m_isRenderingSystem;
	}

protected:
	void renderingSystem( bool set );

private:
	/**
	 * @brief m_isRenderingSystem
	 */
	int m_isRenderingSystem = false;

};

}
