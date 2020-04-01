#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

#define CLOSE_GAME_EVENT    1

class Engine
{
    private:
        static SDL_Window* window;
        static SDL_Renderer* renderer;

    public:
        static void init();
        static void clear(); // TODO: move to display ?
        static void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*);
        static void render();
        static int handleEvents();
        static void destroy();

        static SDL_Renderer* getRenderer() { return renderer; }
};

#endif /* ENGINE_HPP */