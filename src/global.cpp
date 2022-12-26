
#include "global.hpp"

thread_local std::random_device dev;
thread_local std::mt19937 rng(dev());
thread_local std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [0, 1]

float get_random_float()
{
    return dist(rng);
}