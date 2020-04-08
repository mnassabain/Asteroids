#include <Engine.hpp>

GraphicsManager Engine::graphicsManager;
EventManager Engine::eventManager;

void Engine::init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Error SDL_Init(): " << SDL_GetError() << std::endl;
    }

    graphicsManager.init();
}

void Engine::clear()
{
    graphicsManager.clearScreen();
}

void Engine::draw(SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dest)
{
    graphicsManager.draw(texture, src, dest);
}

void Engine::draw(SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dest, int angle)
{
    graphicsManager.draw(texture, src, dest, angle);
}

void Engine::drawRect(const Rect& r)
{
    graphicsManager.drawRect(r);
}

void Engine::render()
{
    graphicsManager.render();
}

void Engine::manageFrames()
{
    SDL_Delay(16); // 1/60 * 1000 = 16ms (60fps)
}

void Engine::destroy() // TODO: move to destructor ?
{
    graphicsManager.destroy();
    SDL_Quit();
}

int Engine::handleEvents()
{
    return eventManager.handleEvents();
}

Event Engine::pollEvent()
{
    return eventManager.poll();
}
