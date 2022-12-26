#include <chrono>
#include <iostream>
#include <memory>

#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().

Scene CreateMISScene(Scene &&scene) {
    auto red = Material::Create(DIFFUSE, Vector3f(0.0f),
                                Vector3f(0.63f, 0.065f, 0.05f));
    auto green = Material::Create(DIFFUSE, Vector3f(0.0f),
                                  Vector3f(0.14f, 0.45f, 0.091f));
    auto white = Material::Create(DIFFUSE, Vector3f(0.0f),
                                  Vector3f(0.725f, 0.71f, 0.68f));
    auto gray =
        Material::Create(GLOSSY, Vector3f(0.0f), Vector3f(0.8f, 0.8f, 0.8f));
    gray->setSpecularExponent(256.f);
    auto light = Material::Create(
        DIFFUSE,
        (5.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) +
         10.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) +
         10.0f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)),
        Vector3f(0.65f));

    auto blinn = Material::Create(GLOSSY, Vector3f(0.0f),
                                  Vector3f(0.725f, 0.71f, 0.68f));
    blinn->setSpecularExponent(4096.f);
    auto green_light =
        Material::Create(DIFFUSE, Vector3f(0.14f, 0.45f, 0.091f), Vector3f());

    auto floor = std::make_unique<MeshTriangle>(
        "../models/cornellbox/floor.obj", white.get());
    auto floor2 = std::make_unique<MeshTriangle>(
        "../models/cornellbox/floor2.obj", blinn.get());
    auto wall = std::make_unique<MeshTriangle>(
        "../models/cornellbox/backwall.obj", blinn.get());
    auto ball1 =
        std::make_unique<Sphere>(Vector3f{300, 250, 150}, 50, light.get());
    // 绿色光源
    auto ball2 = std::make_unique<Sphere>(Vector3f{200, 150, 150}, 40,
                                          green_light.get());
    // 红色光源
    // Sphere ball3({150,250,150},30,new
    // Material(DIFFUSE,Vector3f(0.63f,0.065f,0.05f)));
    auto bunny = std::make_unique<MeshTriangle>("../models/bunny/bunny2.obj",
                                                white.get());
    auto left = std::make_unique<MeshTriangle>("../models/cornellbox/left.obj",
                                               red.get());
    auto right = std::make_unique<MeshTriangle>(
        "../models/cornellbox/right.obj", green.get());

    scene.Add(std::move(floor));
    scene.Add(std::move(floor2));
    scene.Add(std::move(wall));
    scene.Add(std::move(ball1));
    scene.Add(std::move(ball2));
    // scene.Add(&ball3);
    scene.Add(std::move(bunny));
    scene.Add(std::move(left));
    scene.Add(std::move(right));
    
    scene.Add(std::move(red));
    scene.Add(std::move(green));
    scene.Add(std::move(white));
    scene.Add(std::move(gray));
    scene.Add(std::move(light));
    scene.Add(std::move(green_light));
    scene.Add(std::move(blinn));
    return std::move(scene);
}

Scene CreateCornellbox(Scene &&scene) {
    auto red = Material::Create(DIFFUSE, Vector3f(0.0f),
                                Vector3f(0.63f, 0.065f, 0.05f));
    auto green = Material::Create(DIFFUSE, Vector3f(0.0f),
                                  Vector3f(0.14f, 0.45f, 0.091f));
    auto white = Material::Create(DIFFUSE, Vector3f(0.0f),
                                  Vector3f(0.725f, 0.71f, 0.68f));
    auto light = Material::Create(
        DIFFUSE,
        (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) +
         15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) +
         18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)),
        Vector3f(0.65f));

    auto floor = std::make_unique<MeshTriangle>(
        "../models/cornellbox/floor.obj", white.get());
    auto floor2 = std::make_unique<MeshTriangle>(
        "../models/cornellbox/floor2.obj", white.get());
    auto backwall = std::make_unique<MeshTriangle>(
        "../models/cornellbox/backwall.obj", white.get());
    auto shortbox = std::make_unique<MeshTriangle>(
        "../models/cornellbox/shortbox.obj", white.get());
    auto tallbox = std::make_unique<MeshTriangle>(
        "../models/cornellbox/tallbox.obj", white.get());
    auto left = std::make_unique<MeshTriangle>("../models/cornellbox/left.obj",
                                               red.get());
    auto right = std::make_unique<MeshTriangle>(
        "../models/cornellbox/right.obj", green.get());
    auto light_ = std::make_unique<MeshTriangle>(
        "../models/cornellbox/light.obj", light.get());

    scene.Add(std::move(floor));
    scene.Add(std::move(floor2));
    scene.Add(std::move(backwall));
    scene.Add(std::move(shortbox));
    scene.Add(std::move(tallbox));
    scene.Add(std::move(left));
    scene.Add(std::move(right));
    scene.Add(std::move(light_));

    scene.Add(std::move(red));
    scene.Add(std::move(green));
    scene.Add(std::move(white));
    scene.Add(std::move(light));
    return std::move(scene);
}

int main(int argc, char **argv) {
    // Change the definition here to change resolution
    // Scene scene(784, 784);
    // Scene scene(392,392);
    Scene scene(196, 196);
    std::string filename = "binary.ppm";
    if (argc > 1) {
        switch (argv[1][1]) {
        case 'M':
            scene.sample = MIS;
            break;
        case 'B':
            scene.sample = BRDF;
            break;
        case 'L':
            scene.sample = LIGHT;
            break;
        }
        if (argc > 2) {
            filename = argv[2];
        }
    }
    switch (scene.sample) {
    case MIS:
        std::cout << "Sample : MIS\n";
        break;
    case BRDF:
        std::cout << "Sample :BRDF\n";
        break;
    case LIGHT:
        std::cout << "Sample :LIGHT\n";
        break;
    }
    std::cout << "Filename: " << filename << "\n";

    // scene = CreateMISScene(std::move(scene));
    scene = CreateCornellbox(std::move(scene));
    scene.buildBVH();
    scene.initLight();

    Renderer r;
    r.Render(scene, filename);
    return 0;
}