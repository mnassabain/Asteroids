#include <ImageManager.hpp>

std::vector<Image*> ImageManager::images;

void ImageManager::init(int stage)
{
    if (stage == 0)
    {
        // create title images
        images.push_back(new Image("resources/title.png"));
    }
}

void ImageManager::clearImages()
{
    for(auto it = images.begin(); it < images.end(); it++)
    {
        delete (*it);
    }
}

void ImageManager::displayImages()
{
    for(auto it = images.begin(); it < images.end(); it++)
    {
        (*it)->draw();
    }
}
