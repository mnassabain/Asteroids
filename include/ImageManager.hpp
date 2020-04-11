#ifndef IMAGEMANAGER_HPP
#define IMAGEMANAGER_HPP

#include <Image.hpp>
#include <Engine.hpp>

class ImageManager
{
    private:
        static std::vector<Image*> images;
        static int score;
        static void addNumber(int, int);

    public:
        // void createImage(int);
        static void init(int);
        static void clearImages();
        static void displayImages();
        static void updateScore(int);
        static void addScore();
};


#endif /* IMAGEMANAGER_HPP */
