#ifndef GRAPHICSMANAGER_HPP
#define GRAPHICSMANAGER_HPP

#include <iostream>
#include <vector>
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
    
        vector<SDL_Texture*> textures;

    public:
        void init();
        void destroy();

        SDL_Texture* getTexture(int);

        void clearScreen();
        void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*);
        void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*, int);
        void drawRect(const Rect&);
        void render();

        static const int TEXTURE_SPACESHIP = 0;
        static const int TEXTURE_ASTEROID = 1;
        static const int TEXTURE_ROCKET = 2;
        static const int TEXTURE_TITLE = 3;
        static const int TEXTURE_SUBTITLE = 4;
        static const int TEXTURE_NUM = 5;
        static const int TEXTURE_LIVES = 6;
        static SDL_Texture* getTextureFromPath(string path);
};

#endif /* GRAPHICSMANAGER_HPP */