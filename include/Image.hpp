#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <SDL2/SDL.h>
#include <Engine.hpp>
#include <Rect.hpp>

class Image
{
    private:
        SDL_Texture* texture;
        Rect box;
    
    public:
        static const int TITLE_IMAGE_WIDTH = 382;
        static const int TITLE_IMAGE_HEIGHT = 32;

        Image(SDL_Texture*, Rect&);
        void draw();
};

#endif /* IMAGE_HPP */
