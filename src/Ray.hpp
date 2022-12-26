//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_RAY_H
#define RAYTRACING_RAY_H
#include "Vector.hpp"
struct Ray{
    //Destination = origin + t*direction
    Vector3f origin;
    Vector3f direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max;

    Ray(const Vector3f& ori, const Vector3f& dir, const double _t = 0.0): origin(ori), direction(dir),t(_t) {
        direction_inv = Vector3f(1.f/direction.x, 1.f/direction.y, 1.f/direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();
    }

    Vector3f operator()(double t) const{return origin+direction*t;}

    bool operator == (const Ray& r) const{
        return origin == r.origin && direction == r.direction;
    }

    friend std::ostream &operator<<(std::ostream& os, const Ray& r){
        os<<"[origin:="<<r.origin<<", direction="<<r.direction<<", time="<< r.t<<"]\n";
        return os;
    }
};
struct Vector3fHasher {
    std::size_t operator()(const Vector3f& k) const {
        using std::size_t;
        using std::hash;
        using std::string;

        // Compute individual hash values for first,
        // second and third and combine them using XOR
        // and bit shifting:

        return ((hash<float>()(k.x)
                 ^ (hash<float>()(k.y) << 1)) >> 1)
                 ^ (hash<float>()(k.z) << 1);
    }
};
struct RayHasher {
    size_t operator()(const Ray& r) const {
        return Vector3fHasher()(r.origin) ^ Vector3fHasher()(r.direction);
    }
};
#endif //RAYTRACING_RAY_H
