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

SpriteComponent& SpriteComponent::operator= (const SpriteComponent &s)
{
    // TODO: ajouter classe ou SDL_Texture est un sigleton
    // et réécrire cette fonction

    texture = s.texture;

    return *this;
}

void SpriteComponent::draw(Object* o) // TODO: add const ?
{
    Vect2D pos = o->getPosition();
    Vect2D dim = o->getDimensions();
    SDL_Rect dest = { pos.getX(), pos.getY(), dim.getX(), dim.getY() };
    Engine::draw(texture, NULL, &dest, o->getOrientation());
}
