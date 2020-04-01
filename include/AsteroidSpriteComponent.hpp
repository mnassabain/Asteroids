#ifndef ASTEROIDSPRITECOMPONENT_HPP
#define ASTEROIDSPRITECOMPONENT_HPP

#include <string>

#include <SpriteComponent.hpp>

using namespace std;

class AsteroidSpriteComponent : public SpriteComponent
{
    private:
        static const string path;

    public:
        AsteroidSpriteComponent();
};


#endif /* ASTEROIDSPRITECOMPONENT_HPP */
