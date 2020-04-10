#ifndef IMAGEMANAGER_HPP
#define IMAGEMANAGER_HPP

#include <Image.hpp>
#include <Engine.hpp>

class ImageManager
{
    private:
        static std::vector<Image*> images;

    public:
        // void createImage(int);
        static void init(int);
        static void clearImages();
        static void displayImages();
};


#endif /* IMAGEMANAGER_HPP */
