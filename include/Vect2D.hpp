#ifndef VECT2D_HPP
#define VECT2D_HPP

class Vect2D
{
    private:
        int x;
        int y;

    public:
        Vect2D();
        Vect2D(int x, int y);

        int getX();
        void setX(int x);
        int getY();
        void setY(int y);

        Vect2D operator+ (Vect2D const &vector);
        Vect2D operator- (Vect2D const &vector);
        Vect2D& operator= (const Vect2D &vector);
};

#endif /* VECT2D_HPP */