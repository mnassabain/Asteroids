#ifndef VECT2D_H
#define VECT2D_H

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
};

#endif /* VECT2D_H */