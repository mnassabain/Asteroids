#include <SpriteComponent.hpp>
#include <iostream>

#include <Object.hpp>
#include <SDL2/SDL.h>

SpriteComponent::SpriteComponent()
{
    texture = NULL;
    box = Rect(0, 0, -1, -1);
}

void SpriteComponent::draw(Object* o) // TODO: add const ?
{
    Vect2D pos = o->getPosition();
    Vect2D dim = o->getDimensions();
    SDL_Rect dest = { pos.getX(), pos.getY(), dim.getX(), dim.getY() };
    if (box.w() == -1)
    {
        // TODO move sdl rect to graphics component
        Engine::draw(texture, NULL, &dest, o->getOrientation());
    }
    else
    {
        SDL_Rect src = { box.x(), box.y(), box.w(), box.h() };
        Engine::draw(texture, &src, &dest, o->getOrientation());
    }
}
