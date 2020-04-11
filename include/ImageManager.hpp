#ifndef IMAGEMANAGER_HPP
#define IMAGEMANAGER_HPP

#include <Image.hpp>
#include <Engine.hpp>

class ImageManager
{
    private:
        static std::vector<Image*> images;
        static void addNumber(int, int);
        static void addScore(int);

    public:
        // void createImage(int);
        static void init(int);
        static void clearImages();
        static void displayImages();
        static void updateScore(int);
        static void displayScore(int);
};


#endif /* IMAGEMANAGER_HPP */
