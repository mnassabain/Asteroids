#ifndef COLLISIONCOMPONENT_HPP
#define COLLISIONCOMPONENT_HPP

#include <Rect.hpp>

#define COLLISION_ID_SPACESHIP      0b1
#define COLLISION_ID_ASTEROID       0b10
#define COLLISION_ID_ROCKET         0b100

#define COLLISION_MASK_SPACESHIP    0b010
#define COLLISION_MASK_ASTEROID     0b101
#define COLLISION_MASK_ROCKET       0b010

class Object;

class CollisionComponent
{
    protected:
        int collisionId;
        int mask;
        Rect hitbox;
        int colliding; // state: 0 - no collision, else id of colliding object

        void setColliding(int);

    public:
        CollisionComponent();
        CollisionComponent(Rect&);
        int getCollisionId();
        int getMask();
        Rect getHitbox();
        void setHitbox(Rect&);

        bool isCollidingWith(Object*);
        int getColliding();
};

#endif /* COLLISIONCOMPONENT_HPP */
