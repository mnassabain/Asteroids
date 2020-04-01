#include <SpriteComponent.hpp>
#include <iostream>

#include <Object.hpp>

SpriteComponent::SpriteComponent(string path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == NULL)
    {
        std::cout << "Error IMG_Load(): " << SDL_GetError() << std::endl;
    }

    texture = SDL_CreateTextureFromSurface(Engine::getRenderer(), surface);
    SDL_FreeSurface(surface); // TODO: isolate code from graphics component
}

SpriteComponent::~SpriteComponent()
{
    SDL_DestroyTexture(texture);
}

void SpriteComponent::draw(Object* o) // TODO: add const ?
{
    Vect2D pos = o->getPosition();
    SDL_Rect dest = { pos.getX(), pos.getY(), 30, 45 };
    Engine::draw(texture, NULL, &dest);
}
