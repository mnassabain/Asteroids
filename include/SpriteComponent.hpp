#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <iostream>

#include <Vect2D.hpp>
#include <Engine.hpp>

class Object;

class SDL_Texture;

using namespace std;

class SpriteComponent
{
    protected:
        SDL_Texture * texture;  // TODO: remplacer par Resource
        Rect box;               // box on texture
    
    public:
        SpriteComponent();
        void draw(Object*);
};

#endif /* SPRITECOMPONENT_H */