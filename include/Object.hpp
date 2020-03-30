#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "Rect.hpp"

class Object
{
    private:
        Rect hitbox;
        int orientation;    // 0 - 360
        int speed;          // 0 - 100
        int acceleration;   // -1 (deceleration), 0 (no acc), 1 (acceleration)
    
    public:
        Object();

        Rect getHitbox();

        virtual void destroy() = 0;
};

#endif /* OBJECT_HPP */