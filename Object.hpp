//
// Created by LEI XU on 5/13/19.
//
#pragma once
#ifndef RAYTRACING_OBJECT_H
#define RAYTRACING_OBJECT_H

#include "Vector.hpp"
#include "global.hpp"
#include "Bounds3.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Object
{
public:
    Object() = default;
    virtual ~Object() = default;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual Bounds3 getBounds()=0;
    virtual float getArea()=0;
    virtual void Sample(Intersection &pos, float &pdf)=0;
    virtual bool hasEmit()=0;
};



#endif //RAYTRACING_OBJECT_H
