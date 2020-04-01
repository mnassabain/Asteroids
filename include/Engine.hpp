#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <EventManager.hpp>

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

using namespace std;

class Engine
{
    private:
        static SDL_Window* window;
        static SDL_Renderer* renderer;

        static EventManager eventManager;

    public:
        static void init();
        static void clear(); // TODO: move to display ?
        static void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*);
        static void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*, int);
        static void render();
        static int handleEvents();
        static void destroy();

        static SDL_Renderer* getRenderer() { return renderer; }

        static Event pollEvent();
};

#endif /* ENGINE_HPP */