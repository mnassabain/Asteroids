#ifndef GRAPHICSMANAGER_HPP
#define GRAPHICSMANAGER_HPP

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Rect.hpp>

using namespace std;

#define SCREEN_WIDTH    1080
#define SCREEN_HEIGHT   720

class GraphicsManager
{
    private:
        SDL_Window* window;
        static SDL_Renderer* renderer;
    
        SDL_Texture* spaceshipTexture;
        SDL_Texture* asteroidTexture;
        SDL_Texture* rocketTexture;
        SDL_Texture* titleTexture;
    
    public:
        void init();
        void destroy();

        SDL_Texture* getSpaceshipTexture();
        SDL_Texture* getAsteroidTexture();
        SDL_Texture* getRocketTexture();
        SDL_Texture* getTitleTexture();

        void clearScreen();
        void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*);
        void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*, int);
        void drawRect(const Rect&);
        void render();

        static SDL_Texture* getTextureFromPath(string path);
};

#endif /* GRAPHICSMANAGER_HPP */