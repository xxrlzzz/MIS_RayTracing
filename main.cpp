#include <chrono>

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
int main(int argc, char **argv) {
    //  Change the definition here to change resolution
    //  Scene scene(784, 784);
    //  Scene scene(392,392);
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

    auto red = Material::Create(DIFFUSE, Vector3f(0.0f),  Vector3f(0.63f, 0.065f, 0.05f));
    auto green = Material::Create(DIFFUSE, Vector3f(0.0f),Vector3f(0.14f, 0.45f, 0.091f) );
    auto white = Material::Create(DIFFUSE, Vector3f(0.0f), Vector3f(0.725f, 0.71f, 0.68f));
    auto gray = Material::Create(GLOSSY, Vector3f(0.0f), Vector3f(0.8f, 0.8f, 0.8f));
    gray->setSpecularExponent(256.f);
    auto light = Material::Create(
        DIFFUSE, (1.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) +
                  2.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) +
                  2.0f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)), Vector3f(0.65f));

    auto blinn = Material::Create(GLOSSY, Vector3f(0.0f), Vector3f(0.725f, 0.71f, 0.68f));
    blinn->setSpecularExponent(4096.f);

    MeshTriangle floor("../models/cornellbox/floor.obj", white.get());
    MeshTriangle floor2("../models/cornellbox/floor2.obj", blinn.get());
    MeshTriangle wall("../models/cornellbox/backwall.obj", blinn.get());
    Sphere ball1({300, 250, 150}, 50, light.get());
    // 绿色光源
    auto green_light= Material::Create(DIFFUSE, Vector3f(0.14f, 0.45f, 0.091f), Vector3f());
    Sphere ball2({200, 150, 150}, 40,
                  green_light.get());
    // 红色光源
    //    Sphere ball3({150,250,150},30,new
    //    Material(DIFFUSE,Vector3f(0.63f,0.065f,0.05f)));
    MeshTriangle bunny("../models/bunny/bunny2.obj", white.get());
    MeshTriangle left("../models/cornellbox/left.obj", red.get());
    MeshTriangle right("../models/cornellbox/right.obj", green.get());

    scene.Add(&floor);
    scene.Add(&floor2);
    scene.Add(&wall);
    scene.Add(&ball1);
    scene.Add(&ball2);
    //    scene.Add(&ball3);
    scene.Add(&bunny);
    scene.Add(&left);
    scene.Add(&right);

    scene.buildBVH();
    scene.initLight();

    Renderer r;
    r.Render(scene, filename);
    return 0;
}