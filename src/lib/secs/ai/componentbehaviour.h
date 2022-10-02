#pragma once

#include "../secs.h"


namespace secs {


class IComponentBehaviour
{
public:
    virtual void onEnter(const secs::Entity& e, secs::EntityProcessor& processor) = 0;
    virtual void onExit(const secs::Entity& e, secs::EntityProcessor& processor) = 0;
};


template<typename... ComponentTypes>
class ComponentBehaviour : public IComponentBehaviour
{
public:

    virtual void load(const secs::Entity& e, secs::EntityProcessor& processor, ComponentTypes&... args) {}
    virtual void save(const secs::Entity& e, secs::EntityProcessor& processor, ComponentTypes&... args) {}

    void onEnter(const secs::Entity& e, secs::EntityProcessor& processor) override
    {
        load(e, processor, processor.AddOrRetrieveComponent<ComponentTypes...>(e));
    }

    void onExit(const secs::Entity& e, secs::EntityProcessor& processor) override
    {
        save(processor.RemoveComponent<ComponentTypes...>(e));
    }
};


}
