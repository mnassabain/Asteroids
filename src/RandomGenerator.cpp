#include <RandomGenerator.hpp>

int RandomGenerator::seed = 0;

int RandomGenerator::generate()
{
    srand(seed++);
    return rand();
}