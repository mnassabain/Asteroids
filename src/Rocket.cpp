#include <Rocket.hpp>

Rocket::Rocket(Vect2D& startPos, int orientation) 
    : Object(
        new SpriteComponent("resources/rocket2.png"),
        NULL,
        new PhysicsComponent()
    )
{
    hitbox.setDimensions(4, 8);
    hitbox.setPosition(startPos);
    this->orientation = orientation;
}

Rocket::~Rocket()
{}

void Rocket::update()
{
    physicsComponent->update(this);

    int speed = 10;
    int dx, dy;
    double alpha = (orientation % 90) * M_PI / 180;
    if (0 <= orientation && orientation < 90)
    {
        dx = sin(alpha) * speed * 2;
        dy = -cos(alpha) * speed * 2;
    }
    else if (90 <= orientation && orientation < 180)
    {
        dx = cos(alpha) * speed * 2;
        dy = sin(alpha) * speed * 2;
    }
    else if (180 <= orientation && orientation < 270)
    {
        dx = -sin(alpha) * speed * 2;
        dy = cos(alpha) * speed * 2;
    }
    else
    {
        dx = -cos(alpha) * speed * 2;
        dy = -sin(alpha) * speed * 2;
    }

    setPosition(getPosition() + Vect2D(dx, dy));
}
