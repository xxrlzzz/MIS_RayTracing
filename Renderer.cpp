//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0f; }

const float EPSILON = 0.00001;
// const float EPSILON = 0.0001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene,const std::string& file)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tanf(deg2rad(scene.fov * 0.5f));
    float imageAspectRatio = (float)scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;

    // change the spp value to change sample amount
    int spp = 128;
    std::cout << "SPP: " << spp << "\n";

    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5f) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5f) / (float)scene.height) * scale;
            Vector3f dir = normalize(Vector3f(-x, y, 1));
            Vector3f resColor(0.0f);
            Ray ray(eye_pos,dir);
            #pragma omp parallel for shared(spp,scene,resColor)
            for (int k = 0; k < spp; k++){
                resColor += scene.castRay(ray, 0) / (float)spp;
            }
            framebuffer[m] = resColor;
            m++;
        }
        UpdateProgress(j / (float)scene.height);
    }
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen(file.c_str(), "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
