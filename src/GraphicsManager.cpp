#include <GraphicsManager.hpp>

SDL_Texture* GraphicsManager::spaceshipTexture = NULL;
SDL_Texture* GraphicsManager::asteroidTexture = NULL;
SDL_Texture* GraphicsManager::rocketTexture = NULL;

void GraphicsManager::init()
{
    spaceshipTexture = getTextureFromPath("resources/player.png");
    asteroidTexture = getTextureFromPath("resources/asteroid.png");
    rocketTexture = getTextureFromPath("resources/rocket2.png");
}

void GraphicsManager::destroy()
{
    SDL_DestroyTexture(spaceshipTexture);
    SDL_DestroyTexture(asteroidTexture);
    SDL_DestroyTexture(rocketTexture);
}

SDL_Texture* GraphicsManager::getSpaceshipTexture()
{
    return spaceshipTexture;
}

SDL_Texture* GraphicsManager::getAsteroidTexture()
{
    return asteroidTexture;
}

SDL_Texture* GraphicsManager::getRocketTexture()
{
    return rocketTexture;
}

SDL_Texture* GraphicsManager::getTextureFromPath(string path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == NULL)
    {
        cout << "Error IMG_Load(): " << SDL_GetError() << endl;
    }

    SDL_Texture* texture;
    texture = SDL_CreateTextureFromSurface(Engine::getRenderer(), surface);
    SDL_FreeSurface(surface); // TODO: isolate code from graphics component

    return texture;
}
