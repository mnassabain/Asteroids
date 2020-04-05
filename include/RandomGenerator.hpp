#ifndef RANDOMGENERATOR_HPP
#define RANDOMGENERATOR_HPP

#include <cstdlib>

class RandomGenerator
{
    private:
        static int seed;

    public:
        static int generate();
};

#endif /* RANDOMGENERATOR_HPP */
