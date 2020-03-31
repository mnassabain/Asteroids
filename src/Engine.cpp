#include <Engine.hpp>

SDL_Window* Engine::window = NULL;
SDL_Renderer* Engine::renderer = NULL;

void Engine::init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Error SDL_Init(): " << SDL_GetError() << std::endl;
    }

    window = SDL_CreateWindow(
        "Asteroids", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );
    if (window == NULL)
    {
        std::cout << "Error SDL_CreateWindow(): " << SDL_GetError()
            << std::endl;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
}

void Engine::draw()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(16);
}

int Engine::handleEvents()
{
    int result = 0;
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                result = CLOSE_GAME_EVENT;
                break;
        }
    }

    return result;
}

void Engine::destroy()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
