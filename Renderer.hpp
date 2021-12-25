//
// Created by goksu on 2/25/20.
//
#include "Scene.hpp"

#pragma once
class Renderer
{
public:
    void Render(const Scene &scene, const std::string &file = "binary.ppm");
};
