#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>

#include <SDL2/SDL.h>

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

class Engine
{
    private:
        static SDL_Window* window;
        static SDL_Renderer* renderer;

    public:
        static void init();
        static void draw();
        static void destroy();
};

#endif /* ENGINE_HPP */