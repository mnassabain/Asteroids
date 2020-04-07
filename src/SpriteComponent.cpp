#include <SpriteComponent.hpp>
#include <iostream>

#include <Object.hpp>

SpriteComponent::SpriteComponent()
{
    texture = NULL;
}

void SpriteComponent::draw(Object* o) // TODO: add const ?
{
    Vect2D pos = o->getPosition();
    Vect2D dim = o->getDimensions();
    SDL_Rect dest = { pos.getX(), pos.getY(), dim.getX(), dim.getY() };
    Engine::draw(texture, NULL, &dest, o->getOrientation());
}
