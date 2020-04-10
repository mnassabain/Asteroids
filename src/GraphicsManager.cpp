#include <GraphicsManager.hpp>

SDL_Renderer* GraphicsManager::renderer = NULL;

void GraphicsManager::init()
{
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

    if (IMG_Init(IMG_INIT_PNG) < 0)
    {
        std::cout << "error: "<< std::endl;
        exit(1);
    }

    textures.push_back(getTextureFromPath("resources/player.png"));
    textures.push_back(getTextureFromPath("resources/asteroid.png"));
    textures.push_back(getTextureFromPath("resources/rocket2.png"));
    textures.push_back(getTextureFromPath("resources/title.png"));
    textures.push_back(getTextureFromPath("resources/subtitle.png"));
}

void GraphicsManager::destroy()
{
    for (auto it = textures.begin(); it < textures.begin(); it++)
    {
        SDL_DestroyTexture(*it);
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

SDL_Texture* GraphicsManager::getTexture(int code)
{
    return textures[code];
}

SDL_Texture* GraphicsManager::getTextureFromPath(string path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == NULL)
    {
        cout << "Error IMG_Load(): " << SDL_GetError() << endl;
    }

    SDL_Texture* texture;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // TODO: isolate code from graphics component

    return texture;
}

void GraphicsManager::clearScreen()
{
    SDL_SetRenderDrawColor(renderer, 37, 41, 66, 255);
    SDL_RenderClear(renderer);
}

void GraphicsManager::draw(SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dest)
{
    SDL_RenderCopy(renderer, texture, src, dest);
}

void GraphicsManager::draw(SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dest, int angle)
{
    SDL_RenderCopyEx(renderer, texture, src, dest, angle, NULL, SDL_FLIP_NONE);
}

void GraphicsManager::render()
{
    SDL_RenderPresent(renderer);
}

void GraphicsManager::drawRect(const Rect& r)
{
    SDL_Rect rect = { r.x(), r.y(), r.w(), r.h() };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}
