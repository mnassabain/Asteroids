#include <ImageManager.hpp>

std::vector<Image*> ImageManager::images;

void ImageManager::init(int stage)
{
    if (stage == 0)
    {
        // create title images
        Vect2D dim = Vect2D(382, 32); // TODO: MACROS
        Vect2D pos = Vect2D((SCREEN_WIDTH - dim.getX()) / 2,
            (SCREEN_HEIGHT - dim.getY()) / 2 - 50);
        Rect r(pos.getX(), pos.getY(), dim.getX(), dim.getY());
        images.push_back(new Image(Engine::getTitleTexture(), r));
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
