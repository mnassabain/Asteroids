#ifndef GRAPHICSMANAGER_HPP
#define GRAPHICSMANAGER_HPP

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Engine.hpp>

using namespace std;

class GraphicsManager
{
    private:
        static SDL_Texture* spaceshipTexture;
        static SDL_Texture* asteroidTexture;
        static SDL_Texture* rocketTexture;

        static SDL_Texture* getTextureFromPath(string path);
    
    public:
        static void init();
        static void destroy();

        static SDL_Texture* getSpaceshipTexture();
        static SDL_Texture* getAsteroidTexture();
        static SDL_Texture* getRocketTexture();
};

#endif /* GRAPHICSMANAGER_HPP */