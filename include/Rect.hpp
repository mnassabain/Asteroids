#ifndef RECT_HPP
#define RECT_HPP

#include "Vect2D.hpp"

class Rect
{
    private:
        Vect2D position;
        Vect2D dimensions;  // x = width, y = height

    public:
        Rect() {}
        Rect(const Rect& r);
        Rect(Vect2D pos, Vect2D dim);
        Rect(int x, int y, int w, int h);

        Rect& operator= (const Rect&);

        Vect2D getPosition();
        void setPosition(Vect2D pos);
        void setPosition(int x, int y);

        int x() const;
        int y() const;
        int w() const;
        int h() const;

        void x(int);
        void y(int);
        void w(int);
        void h(int);

        Vect2D getDimensions();
        void setDimensions(Vect2D dim);
        void setDimensions(int w, int h);
        void setWidth(int w);
        void setHeight(int h);
        int getWidth();
        int getHeight();
        void set(int x, int y, int w, int h);

        // TODO: static test collision
};

#endif /* RECT_HPP */