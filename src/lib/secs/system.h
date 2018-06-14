#pragma once

namespace secs
{

class System
{
public:

	typedef System* Ptr;

	virtual ~System();

    virtual void step( double delta );

    virtual void renderStep();

    virtual void preUpdate( double delta );

    virtual void postUpdate( double delta );

    void setStepConfiguration( bool process_step, bool render_step );

    void enable();

    void disable();

    bool isEnabled();

    bool hasProcessingStep();

    bool hasRenderingStep();

    int executionPriority();

protected:

    void setExecutionPriority(int priority);

private:
    bool m_hasProcessingStep = true;
    bool m_hasRenderingStep = false;
    bool m_isEnabled = true;
    int m_executionPriority = 0;

};

}
