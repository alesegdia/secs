#include "system.h"

#include "config.h"

secs::System::~System()
= default;

void secs::System::Step(double delta)
{
    SECS_UNUSED(delta);
}

void secs::System::RenderStep()
{

}

void secs::System::PreUpdate(double delta)
{
    SECS_UNUSED(delta);
}

void secs::System::PostUpdate(double delta)
{
    SECS_UNUSED(delta);
}

void secs::System::SetStepConfiguration(bool process_step, bool render_step)
{
    m_hasProcessingStep = process_step;
    m_hasRenderingStep = render_step;
}

void secs::System::Enable()
{
    m_isEnabled = true;
}

void secs::System::Disable()
{
    m_isEnabled = false;
}

bool secs::System::IsEnabled()
{
    return m_isEnabled;
}

bool secs::System::HasProcessingStep()
{
    return m_hasProcessingStep;
}

bool secs::System::HasRenderingStep()
{
    return m_hasRenderingStep;
}

int secs::System::GetExecutionPriority()
{
    return m_executionPriority;
}

void secs::System::SetExecutionPriority(int priority)
{
    m_executionPriority = priority;
}

