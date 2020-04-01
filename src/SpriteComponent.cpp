#include <SpriteComponent.hpp>
#include <iostream>

SpriteComponent::SpriteComponent()
{
    SDL_Surface* surface = IMG_Load("resources/player.png");
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

void SpriteComponent::draw()
{
    Engine::draw(texture, NULL, NULL);
}
