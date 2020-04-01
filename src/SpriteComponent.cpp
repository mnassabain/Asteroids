#include <SpriteComponent.hpp>
#include <iostream>

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

void SpriteComponent::draw()
{
    SDL_Rect dest = { 300, 200, 30, 45 };
    Engine::draw(texture, NULL, &dest);
}
