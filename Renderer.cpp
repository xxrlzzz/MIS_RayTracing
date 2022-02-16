//
// Created by goksu on 2/25/20.
//

#include <array>
#include <fstream>
#include <future>
#include <thread>
#include <vector>

#include "Profiler.h"
#include "Renderer.hpp"
#include "Scene.hpp"

inline float deg2rad(const float &deg) { return deg * M_PI / 180.0f; }

constexpr int kSPP = 64;
constexpr int kBatchSize = 4;

const float EPSILON = 0.00001;
// const float EPSILON = 0.0001;

// save framebuffer to file
void saveFramebuffer(const std::string &file, size_t width, size_t height,
                     const std::vector<Vector3f> &framebuffer) {

    FILE *fp = fopen(file.c_str(), "wb");
    assert(fp != nullptr);
    (void)fprintf(fp, "P6\n%zu %zu\n255\n", width, height);
    for (auto i = 0; i < height * width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x),
                                                  0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y),
                                                  0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z),
                                                  0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}

void Renderer::RenderMain(Scene const *scene, const float scale,
                          const float imageAspectRatio,
                          const Vector3f &eye_pos) {
    // std::cout << "start render thread " << std::this_thread::get_id()
    //           << std::endl;
    std::array<Vector3f, 1024> resColorBuf;
    size_t m = 0;
    for (uint32_t j = 0; j < scene->height; ++j) {
        for (uint32_t i = 0; i < scene->width; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5f) / (float)scene->width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5f) / (float)scene->height) * scale;
            Vector3f dir = normalize(Vector3f(-x, y, 1));
            Vector3f resColor(0.0f);
            Ray ray(eye_pos, dir);
            for (int _ = 0; _ < kBatchSize; _++) {
                resColor += scene->castRay(ray) / (float)kSPP;
            }
            resColorBuf[i] = resColor;
        }
        {
            auto lock = std::scoped_lock(m_framebufferMutex);
            for (uint32_t i = 0; i < scene->width; ++i) {
                m_framebuffer[m] += resColorBuf[i];
                m += 1;
            }
        }
    }
}

void Renderer::doRender(const Scene &scene) {
    const float scale = tanf(deg2rad(scene.fov * 0.5f));
    const float imageAspectRatio = (float)scene.width / (float)scene.height;
    const Vector3f eye_pos(278, 273, -800);

    std::vector<std::thread> renderers = {};
    for (int k = 0; k < kSPP; k += kBatchSize) {
        renderers.emplace_back(&Renderer::RenderMain, this, &scene, scale,
                               imageAspectRatio, eye_pos);
    }

    for (auto &&th : renderers) {
        th.join();
    }
    UpdateProgress(1.f);
}

void Renderer::WindowMain(const std::string &file, size_t width,
                          size_t height) {
    sf::Event event{};
    while (m_window.isOpen()) {
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape)) {
                m_window.close();
                exit = true;
                return;
            }

            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Q:
                    next_sample = SAMPLE::BRDF;
                    std::cout << "next sample: BRDF" << std::endl;
                    break;
                case sf::Keyboard::W:
                    next_sample = SAMPLE::MIS;
                    std::cout << "next sample: MIS" << std::endl;
                    break;
                case sf::Keyboard::E:
                    next_sample = SAMPLE::LIGHT;
                    std::cout << "next sample: LIGHT" << std::endl;
                    break;
                case sf::Keyboard::Up:
                    next_rate = std::min(next_rate + 0.05f, 1.f);
                    std::cout << "next rate: " << next_rate << std::endl;
                    break;
                case sf::Keyboard::Down:
                    next_rate = std::max(next_rate - 0.05f, 0.f);
                    std::cout << "next rate: " << next_rate << std::endl;
                    break;
                case sf::Keyboard::S:
                    next_save = !next_save;
                    std::cout << "next save: " << next_save << std::endl;
                    break;
                default:
                    break;
                }
            }
        }

        if (next_save) {
            saveFramebuffer(file, width, height, m_framebuffer_copy);
            next_save = false;
        }
        size_t m = 0;
        for (uint32_t i = 0; i < width; ++i) {
            for (uint32_t j = 0; j < height; ++j) {
                const Vector3f color =
                    Vector3f::Min(m_framebuffer_copy[m], Vector3f{1, 1, 1});
                m_screen.setPixel(j, i,
                                  sf::Color(color.x * 255, color.y * 255,
                                            color.z * 255, 0xFF));
                m += 1;
            }
        }
        m_window.draw(m_screen);
        m_window.display();

        std::this_thread::sleep_for(std::chrono::microseconds(17));
    }
}

// The main render function. This where we iterate over all pixels in the
// image, generate primary rays and cast these rays into the scene. The
// content of the framebuffer is saved to a file.
void Renderer::Render(Scene &scene, const std::string &file) {
    m_window.create(sf::VideoMode(scene.width, scene.height), "Render",
                    sf::Style::Titlebar | sf::Style::Close);
    m_window.setVerticalSyncEnabled(true);
    m_screen.create(scene.width, scene.height, 1.f, sf::Color::White);
    const size_t framebufferSize = scene.width * scene.height;
    m_framebuffer.resize(framebufferSize);
    m_framebuffer_copy.resize(framebufferSize);
    size_t m = 0;
    for (uint32_t i = 0; i < scene.width; ++i) {
        for (uint32_t j = 0; j < scene.height; ++j) {
            m_screen.setPixel(j, i, sf::Color::White);
            m += 1;
        }
    }
    next_rate = scene.mis_rate;
    next_sample = scene.sample;

    // change the spp value to change sample amount
    std::cout << "SPP: " << kSPP << "\n";
    std::thread th = std::thread([&scene, this, framebufferSize]() {
        while (!exit) {
            {
                RAIIProfiler profiler;
                this->doRender(scene);
                std::cout << "\n";
            }

            {
                auto lock = std::scoped_lock(m_framebufferMutex);
                m_framebuffer_copy = m_framebuffer;
                m_framebuffer.clear();
                m_framebuffer.resize(framebufferSize);
            }

            while (!exit && !scene.UpdateRenderConfig(next_sample, next_rate)) {
                std::this_thread::yield();
            }
            if (!exit) {
                std::cout << "Update sample: " << next_sample
                          << " rate: " << next_rate << "\n";
            }
        }
    });

    WindowMain(file, scene.width, scene.height);
    th.join();
}
