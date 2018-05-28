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

    void enable()
    {
        m_isEnabled = true;
    }

    void disable()
    {
        m_isEnabled = false;
    }

    bool isEnabled()
    {
        return m_isEnabled;
    }

protected:
    void renderingSystem( bool set );

private:
    /**
     * @brief m_isRenderingSystem
     */
    int m_isRenderingSystem = false;

    bool m_isEnabled = true;

};

}
