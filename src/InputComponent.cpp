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
                ship->accelerate();
                break;

            case EVT_PLAYER_DECELERATE:
                ship->decelerate();
                break;

            case EVT_PLAYER_START_TURN_CCW:
                ship->startTurning(COUNTERCLOCKWISE);
                break;

            case EVT_PLAYER_START_TURN_CW:
                ship->startTurning(CLOCKWISE);
                break;

            case EVT_PLAYER_STOP_TURN:
                ship->stopTurning();
                break;
        }
    }
}
