#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Engine.hpp>

class SpriteComponent
{
    private:
        SDL_Texture * texture; // TODO: remplacer par Resource
    
    public:
        SpriteComponent();
        ~SpriteComponent();
        void draw();
};

#endif /* SPRITECOMPONENT_H */