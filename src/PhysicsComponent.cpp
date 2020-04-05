#include <PhysicsComponent.hpp>
#include <Object.hpp>

int PhysicsComponent::WORLD_WIDTH = SCREEN_WIDTH;
int PhysicsComponent::WORLD_HEIGHT = SCREEN_HEIGHT;

PhysicsComponent::~PhysicsComponent()
{}

bool PhysicsComponent::outOfBounds(Object* o)
{
    return (o->getX() > WORLD_WIDTH)
        || (o->getX() + o->getW() < 0)
        || (o->getY() > WORLD_HEIGHT)
        || (o->getY() + o->getH() < 0);
}

void PhysicsComponent::update(Object* o)
{
    if (outOfBounds(o))
    {
        // circular world
        o->setPosition((o->getX() + WORLD_WIDTH) % WORLD_WIDTH, 
            (o->getY() + WORLD_HEIGHT) % WORLD_HEIGHT);
    }
}
