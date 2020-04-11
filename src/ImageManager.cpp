#include <ImageManager.hpp>

std::vector<Image*> ImageManager::images;

#include <cmath>

void ImageManager::addNumber(int n, int pos)
{
    Rect box(35 + pos*18*2 + pos*5, 30, 18*2, 16*2);
    Rect surface(n*18, 0, 18, 16);
    images.push_back(new Image(Engine::getTexture(GraphicsManager::TEXTURE_NUM),
        box, surface));
}

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
    images.clear();
}

void ImageManager::displayImages()
{
    for(auto it = images.begin(); it < images.end(); it++)
    {
        (*it)->draw();
    }
}

void ImageManager::displayScore(int score)
{
    clearImages();
    int pos = 0;
    int s = score;
    // cout << "adding nums - score: " << score << endl;
    int maxpow = floor(log10(s));
    while(maxpow >= 0)
    {
        int plusFort = s / pow(10, maxpow);
        addNumber(plusFort, pos);
        s = s - plusFort * pow(10, maxpow);
        pos++;
        maxpow--;
    }
    if (pos == 0)
    {
        addNumber(0, 0);
    }
}

void ImageManager::addLife(bool x, int pos)
{
    int n;
    if (x) n = 0;
    else n = 1;
    Rect box(35 + pos*18*2 + pos*5, 77, 18*2, 16*2); // TODO: macros
    Rect surface(n*21, 0, 21, 18);
    images.push_back(
        new Image(
            Engine::getTexture(GraphicsManager::TEXTURE_LIVES),
            box,
            surface
        )
    );
}

void ImageManager::displayLives(int lives)
{
    int l = 0;
    int pos = 0;
    while (l < lives)
    {
        addLife(true, pos);
        l++;
        pos++;
    }
    while (l < 5)
    {
        addLife(false, pos);
        l++;
        pos++;
    }
}
