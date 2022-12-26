//
// Created by mis on 2020/4/30.
//

#include "VirtualScreen.hpp"

void VirtualScreen::create(unsigned int w, unsigned int h, float pixel_size,
                           sf::Color color) {
    m_vertices.resize(w * h * 6);
    m_screenSize = {w, h};
    m_vertices.setPrimitiveType(sf::Triangles);
    m_pixelSize = pixel_size;
    for (auto x = 0u; x < w; ++x) {
        for (auto y = 0u; y < h; ++y) {
            auto index = (x * m_screenSize.y + y) * 6;
            sf::Vector2f coord2d(x * m_pixelSize, y * m_pixelSize);

            // Triangle-1
            // top-left
            m_vertices[index].position = coord2d;
            m_vertices[index].color = color;

            // top-right
            m_vertices[index + 1].position =
                coord2d + sf::Vector2f{m_pixelSize, 0};
            m_vertices[index + 1].color = color;

            // bottom-right
            m_vertices[index + 2].position =
                coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
            m_vertices[index + 2].color = color;

            // 2
            // bottom-right
            m_vertices[index + 3].position =
                coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
            m_vertices[index + 3].color = color;

            // bottom-left
            m_vertices[index + 4].position =
                coord2d + sf::Vector2f{0, m_pixelSize};
            m_vertices[index + 4].color = color;

            // top-light
            m_vertices[index + 5].position = coord2d;
            m_vertices[index + 5].color = color;
        }
    }
}

void VirtualScreen::fillPixel(std::vector<sf::Color> colors) {
    int m = 0;
    for (int i = 0; i < colors.size(); ++i) {
        for (int j = 0; j < 6; ++j) {
            m_vertices[m++].color = colors[i];
        }
    }
}

void VirtualScreen::setPixel(std::size_t x, std::size_t y, sf::Color color) {
    auto index = (x * m_screenSize.y + y) * 6;
    if (index >= m_vertices.getVertexCount())
        return;

    for (int i = 0; i < 6; ++i) {
        m_vertices[index + i].color = color;
    }
}

void VirtualScreen::draw(sf::RenderTarget &target,
                         sf::RenderStates states) const {
    target.draw(m_vertices, states);
}
