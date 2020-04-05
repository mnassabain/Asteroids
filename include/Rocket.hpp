#ifndef ROCKET_HPP
#define ROCKET_HPP

#include <Vect2D.hpp>
#include <Object.hpp>
#include <RocketPhysicsComponent.hpp>

class Rocket : public Object
{
    public:
        Rocket(Vect2D& startPos, int orientation);
        ~Rocket();
        void update();
};

#endif /* ROCKET_HPP */