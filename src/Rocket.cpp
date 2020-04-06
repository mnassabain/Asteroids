#include <Rocket.hpp>

Rocket::Rocket(Vect2D& startPos, int orientation) 
    : Object(
        new SpriteComponent("resources/rocket2.png"),
        NULL,
        new RocketPhysicsComponent(),
        new RocketCollisionComponent()
    )
{
    hitbox.setDimensions(4, 8);
    hitbox.setPosition(startPos);
    this->orientation = orientation;
    speed = 10;

    Rect colliderBox(
        hitbox.x(),
        hitbox.y(),
        hitbox.w(),
        hitbox.h()
    );
    collisionComponent->setHitbox(colliderBox);
}

Rocket::~Rocket()
{}

void Rocket::update()
{
    physicsComponent->update(this);
}
