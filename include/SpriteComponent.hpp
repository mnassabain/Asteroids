#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <iostream>
using namespace std;

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Vect2D.hpp>
#include <Engine.hpp>

class Object;

class SpriteComponent
{
    private:
        SDL_Texture * texture; // TODO: remplacer par Resource
    
    public:
        SpriteComponent(string path);
        ~SpriteComponent();
        SpriteComponent& operator= (const SpriteComponent &s);
        void draw(Object*);
};

#endif /* SPRITECOMPONENT_H */