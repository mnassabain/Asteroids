#ifndef SPACESHIPSPRITECOMPONENT_HPP
#define SPACESHIPSPRITECOMPONENT_HPP

#include <string>
using namespace std;

#include <SpriteComponent.hpp>

class SpaceshipSpriteComponent : public SpriteComponent
{
    private:
        static const string path;

    public:
        SpaceshipSpriteComponent();
};


#endif /* SPACESHIPSPRITECOMPONENT_HPP */
