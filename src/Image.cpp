#include <Image.hpp>

Image::Image(string path)
{
    texture = GraphicsManager::getTextureFromPath(path);
}

Image::~Image()
{
    SDL_DestroyTexture(texture);
}

void Image::draw()
{
    Vect2D dim = Vect2D(382, 32); // TODO: MACROS
    Vect2D pos = Vect2D((SCREEN_WIDTH - dim.getX()) / 2,
        (SCREEN_HEIGHT - dim.getY()) / 2 - 50);
    SDL_Rect dest = { pos.getX(), pos.getY(), dim.getX(), dim.getY() };
    Engine::draw(texture, NULL, &dest, 0);
}
