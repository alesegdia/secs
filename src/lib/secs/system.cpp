#include "system.h"

#include "config.h"

secs::System::~System()
{

}

void secs::System::step(double delta)
{
    SECS_UNUSED(delta);
}

void secs::System::renderStep()
{

}

void secs::System::preUpdate(double delta)
{
    SECS_UNUSED(delta);
}

void secs::System::postUpdate(double delta)
{
    SECS_UNUSED(delta);
}

void secs::System::setStepConfiguration(bool process_step, bool render_step)
{
    m_hasProcessingStep = process_step;
    m_hasRenderingStep = render_step;
}

void secs::System::enable()
{
    m_isEnabled = true;
}

void secs::System::disable()
{
    m_isEnabled = false;
}

bool secs::System::isEnabled()
{
    return m_isEnabled;
}

bool secs::System::hasProcessingStep()
{
    return m_hasProcessingStep;
}

bool secs::System::hasRenderingStep()
{
    return m_hasRenderingStep;
}

int secs::System::executionPriority()
{
    return m_executionPriority;
}

void secs::System::setExecutionPriority(int priority)
{
    m_executionPriority = priority;
}

