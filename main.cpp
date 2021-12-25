#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>


// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char **argv) {
  //     Change the definition here to change resolution
  //  Scene scene(784, 784);
  //    Scene scene(392,392);
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
    printf("Sample : MIS\n");
    break;
  case BRDF:
    printf("Sample :BRDF\n");
    break;
  case LIGHT:
    printf("Sample :LIGHT\n");
    break;
  }
  std::cout << "Filename: " << filename << "\n";

  auto *red = new Material(DIFFUSE, Vector3f(0.0f));
  red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
  auto *green = new Material(DIFFUSE, Vector3f(0.0f));
  green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
  auto *white = new Material(DIFFUSE, Vector3f(0.0f));
  white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
  auto *gray = new Material(GLOSSY, Vector3f(0.0f));
  gray->Kd = Vector3f(0.8f, 0.8f, 0.8f);
  gray->specularExponent = 256.0f;
  auto *light = new Material(
      DIFFUSE, (1.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) +
                2.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) +
                2.0f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
  light->Kd = Vector3f(0.65f);

  auto *blinn = new Material(GLOSSY, Vector3f(0.0f));
  blinn->Kd = Vector3f(0.725f, 0.71f, 0.68f);
  blinn->specularExponent = 4096.0f;

  MeshTriangle floor("../models/cornellbox/floor.obj", white);
  MeshTriangle floor2("../models/cornellbox/floor2.obj", blinn);
  MeshTriangle wall("../models/cornellbox/backwall.obj", blinn);
  Sphere ball1({300, 250, 150}, 50, light);
  // 绿色光源
  Sphere ball2({200, 150, 150}, 40,
               new Material(DIFFUSE, Vector3f(0.14f, 0.45f, 0.091f)));
  // 红色光源
  //    Sphere ball3({150,250,150},30,new
  //    Material(DIFFUSE,Vector3f(0.63f,0.065f,0.05f)));
  MeshTriangle bunny("../models/bunny/bunny2.obj", white);
  MeshTriangle left("../models/cornellbox/left.obj", red);
  MeshTriangle right("../models/cornellbox/right.obj", green);

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

  auto start = std::chrono::system_clock::now();
  r.Render(scene, filename);
  auto stop = std::chrono::system_clock::now();

  std::cout << "Render complete: \n";
  std::cout
      << "Time taken: "
      << std::chrono::duration_cast<std::chrono::hours>(stop - start).count()
      << " hours\n";
  std::cout
      << "          : "
      << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count()
      << " minutes\n";
  std::cout
      << "          : "
      << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
      << " seconds\n";

  return 0;
}