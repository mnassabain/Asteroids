#ifndef INPUTCOMPONENT_HPP
#define INPUTCOMPONENT_HPP

#include <SDL2/SDL.h>

#include <Engine.hpp>

#define PLAYER_MOVE_UP_EVENT    2

class Spaceship;

class InputComponent
{
    public:
        void update(Spaceship*);
};

#endif /* INPUTCOMPONENT_HPP */
