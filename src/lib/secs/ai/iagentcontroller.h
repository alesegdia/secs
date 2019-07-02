
#pragma once

#include "componentbehaviour.h"


namespace secs {


class IAgentController
{
public:
    virtual ~IAgentController();
    virtual IComponentBehaviour* process(const secs::Entity& e, secs::EntityProcessor& processor) = 0;
};


}
