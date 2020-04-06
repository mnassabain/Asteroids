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
        Vect2D(const Vect2D &v);

        int getX() const;
        void setX(int x);
        int getY() const;
        void setY(int y);

        Vect2D operator+ (Vect2D const &vector);
        Vect2D operator- (Vect2D const &vector);
        Vect2D& operator= (const Vect2D &vector);
};

#endif /* VECT2D_HPP */