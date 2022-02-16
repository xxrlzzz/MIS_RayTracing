//
// Created by goksu on 2/25/20.
//

#ifndef RENDERER_H 
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include <mutex>
#include <string>

#include "Scene.hpp"
#include "VirtualScreen.hpp"

class Renderer {
  public:
    void Render(Scene &scene, const std::string &file = "binary.ppm");

  private:
    void doRender(const Scene &scene);
    void WindowMain(const std::string &file, size_t width, size_t height);
    void RenderMain(Scene const *scene, const float scale,
                    const float imageAspectRatio, const Vector3f &eye_pos);

    std::vector<Vector3f> m_framebuffer;
    std::vector<Vector3f> m_framebuffer_copy;
    sf::RenderWindow m_window;
    VirtualScreen m_screen;
    std::mutex m_framebufferMutex;

    // Keyboard control.
    std::atomic<bool> exit = false;
    SAMPLE next_sample;
    float next_rate;
    bool next_save = false;
};

#endif // RENDERER_H
