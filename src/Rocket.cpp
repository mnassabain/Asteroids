#include "Rocket.hpp"

Rocket::Rocket() : Object(new SpriteComponent("resources/rocket2.png"))
{
    hitbox.setDimensions(4, 8);
    hitbox.setPosition(200, 400);
    orientation = 58;
}

void Rocket::update()
{

}
