#pragma once

#include <memory>

namespace secs
{

class System
{
public:

    typedef std::shared_ptr<System> Ptr;

	virtual ~System();

    virtual void Step( double delta );

    virtual void RenderStep();

    virtual void PreUpdate( double delta );

    virtual void PostUpdate( double delta );

    void SetStepConfiguration( bool process_step, bool render_step );

    void Enable();

    void Disable();

    bool IsEnabled();

    bool HasProcessingStep();

    bool HasRenderingStep();

    int GetExecutionPriority();

protected:

    void SetExecutionPriority(int priority);

private:
    bool m_hasProcessingStep = true;
    bool m_hasRenderingStep = false;
    bool m_isEnabled = true;
    int m_executionPriority = 0;

};

}
