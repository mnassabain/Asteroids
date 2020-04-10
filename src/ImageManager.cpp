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
        images.push_back(
            new Image(Engine::getTexture(GraphicsManager::TEXTURE_TITLE), r)
        );

        Vect2D dim2 = Vect2D(393, 16);
        Vect2D pos2 = pos + Vect2D(0, dim.getY() + 25);
        Rect r2(pos2.getX(), pos2.getY(), dim2.getX(), dim2.getY());
        images.push_back(
            new Image(Engine::getTexture(GraphicsManager::TEXTURE_SUBTITLE), r2)
        );
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
