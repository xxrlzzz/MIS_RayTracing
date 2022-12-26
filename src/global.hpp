
#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>
#include <cmath>
#include <random>

extern const float EPSILON;
const float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float &lo, const float &hi, const float &v)
{ return std::max(lo, std::min(hi, v)); }

inline  bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5f * b / a;
    else {
        discr = sqrtf(discr);
        float base = 0.5f/a;
        x0 = (-b + discr) * base;
        x1 = (-b - discr) * base;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}

extern float get_random_float();

inline void UpdateProgress(float progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};

#endif // GLOBAL_H
