#include <InputComponent.hpp>

#include <Engine.hpp>
#include <Spaceship.hpp>

void InputComponent::update(Spaceship* ship)
{
    while(!Engine::events.empty())
    {
        int event;
        switch(event = Engine::events.front())
        {
            case PLAYER_MOVE_UP_EVENT:
                ship->setPosition(ship->getPosition() + Vect2D(0, 10));
                break;
        }
        Engine::events.pop();
    }
}
