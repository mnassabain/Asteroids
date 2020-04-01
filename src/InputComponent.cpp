#include <InputComponent.hpp>

#include <Engine.hpp>
#include <Spaceship.hpp>

void InputComponent::update(Spaceship* ship)
{
    Event event;
    while((event = Engine::pollEvent()))
    {
        switch(event)
        {
            case EVT_PLAYER_ACCELERATE:
                ship->setPosition(ship->getPosition() + Vect2D(0, 10));
                break;
        }
    }
}
