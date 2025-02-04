#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <GraphicsManager.hpp>
#include <EventManager.hpp>

using namespace std;

class Engine
{
    private:
        static GraphicsManager graphicsManager;
        static EventManager eventManager;

    public:
        static void init();
        static void clear(); // TODO: move to display ?
        static void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*);
        static void draw(SDL_Texture*, SDL_Rect*, SDL_Rect*, int);
        static void render();
        static void manageFrames();
        static int handleEvents();
        static void destroy();

        static void drawRect(const Rect&);

        static Event pollEvent();
        static void clearEvents();

        static SDL_Texture* getTexture(int code) { return graphicsManager.getTexture(code); }
};

#endif /* ENGINE_HPP */