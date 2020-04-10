#include <Image.hpp>

Image::Image(SDL_Texture* t, Rect& b)
{
    texture = t;
    box = b;
}

void Image::draw()
{
    SDL_Rect dest = { box.x(), box.y(), box.w(), box.h() };
    Engine::draw(texture, NULL, &dest, 0);
}
