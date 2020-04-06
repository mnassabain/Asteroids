#include <RocketPhysicsComponent.hpp>
#include <Object.hpp>

void RocketPhysicsComponent::update(Object* o)
{
    move(o);
    
    if (outOfBounds(o))
    {
        o->destroy();
    }
}
