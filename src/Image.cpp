#include <Image.hpp>

Image::Image(SDL_Texture* t, Rect& b) : surface(0, 0, -1, -1)
{
    texture = t;
    box = b;
}

Image::Image(SDL_Texture* t, Rect& b, Rect& s)
{
    texture = t;
    box = b;
    surface = s;
}

void Image::draw()
{
    SDL_Rect dest = { box.x(), box.y(), box.w(), box.h() };
    if (surface.w() == -1)
    {
        Engine::draw(texture, NULL, &dest, 0);
    }
    else
    {
        SDL_Rect surf = { surface.x(), surface.y(), surface.w(), surface.h() };
        Engine::draw(texture, &surf, &dest, 0);
    }
    
}
