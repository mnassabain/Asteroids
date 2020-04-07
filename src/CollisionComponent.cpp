#include <CollisionComponent.hpp>
#include <Object.hpp>

CollisionComponent::CollisionComponent()
{
    hitbox.x(0);
    hitbox.y(0);
    hitbox.w(0);
    hitbox.h(0);
}

CollisionComponent::CollisionComponent(Rect& h) : hitbox(h)
{}

CollisionComponent::CollisionComponent(const CollisionComponent& c)
{
    collisionId = c.collisionId;
    mask = c.mask;
    hitbox = c.hitbox;
    colliding = c.colliding;
}

CollisionComponent::~CollisionComponent()
{}

CollisionComponent& CollisionComponent::operator=(const CollisionComponent& c)
{
    collisionId = c.collisionId;
    mask = c.mask;
    hitbox = c.hitbox;
    colliding = c.colliding;

    return *this;
}

int CollisionComponent::getCollisionId()
{
    return collisionId;
}

int CollisionComponent::getMask()
{
    return mask;
}

Rect CollisionComponent::getHitbox()
{
    return hitbox;
}

void CollisionComponent::setHitbox(Rect& h)
{
    hitbox = h;
}

bool CollisionComponent::isCollidingWith(Object* o)
{
    bool result = false;

    CollisionComponent * cc2 = o->getCollisionComponent();

    // check masks
    if (mask & cc2->getCollisionId())
    {   
        // AABB
        Rect hitbox2 = cc2->getHitbox();
        if (hitbox.x() < hitbox2.x() + hitbox2.w() &&
            hitbox.x() + hitbox.w() > hitbox2.x() &&
            hitbox.y() < hitbox2.y() + hitbox2.h() &&
            hitbox.y() + hitbox.h() > hitbox2.y()
        )
        {
            result = true;
            colliding = cc2->getCollisionId();
            cc2->setColliding(collisionId);
        }
    }
    
    return result;
}

void CollisionComponent::setColliding(int c)
{
    colliding = c;
}

int CollisionComponent::getColliding()
{
    return colliding;
}
