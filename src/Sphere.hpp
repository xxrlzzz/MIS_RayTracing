//
// Created by LEI XU on 5/13/19.
//

#ifndef RAYTRACING_SPHERE_H
#define RAYTRACING_SPHERE_H

#include "Bounds3.hpp"
#include "Material.hpp"
#include "Object.hpp"
#include "Vector.hpp"

class Sphere : public Object {
  private:
    Vector3f center;
    float radius, radius2;
    Material *m;
    float area;

  public:
    Sphere(const Vector3f &c, const float &r, Material *mt)
        : center(c), radius(r), radius2(r * r), m(mt), area(4 * M_PI * r * r) {}
    Intersection getIntersection(Ray ray) const override {
        Intersection result;
        result.happened = false;
        Vector3f L = ray.origin - center;
        if (L.norm() < radius)
            return result;
        float a = dotProduct(ray.direction, ray.direction);
        float b = 2 * dotProduct(ray.direction, L);
        float c = dotProduct(L, L) - radius2;
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1))
            return result;
        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
            return result;
        result.happened = true;

        result.coords = Vector3f(ray.origin + ray.direction * t0);
        result.normal = normalize(Vector3f(result.coords - center));
        result.m = this->m;
        result.obj = this;
        result.distance = t0;
        return result;
    }

    Bounds3 getBounds() const override {
        return Bounds3(
            Vector3f(center.x - radius, center.y - radius, center.z - radius),
            Vector3f(center.x + radius, center.y + radius, center.z + radius));
    }
    void Sample(Intersection &pos, float &pdf) const override {
        float theta = 2.0f * M_PI * get_random_float(),
              phi = M_PI * get_random_float();
        Vector3f dir(std::cos(phi), std::sin(phi) * std::cos(theta),
                     std::sin(phi) * std::sin(theta));
        pos.coords = center + radius * dir;
        pos.normal = dir;
        pos.emit = m->getEmission();
        pdf = 1.0f / area;
    }
    float getArea() const override { return area; }
    bool hasEmit() const override { return m->hasEmission(); }
    Vector3f getEmission() const { return m->getEmission(); }
    Vector3f getCenter() const {
        return center;
    }

    constexpr float getRadius2() const {
        return radius2;
    }
};

#endif // RAYTRACING_SPHERE_H
