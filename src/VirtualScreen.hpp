//
// Created by mis on 2020/4/29.
//

#ifndef SPNES_VIRTUALSCREEN_H
#define SPNES_VIRTUALSCREEN_H
#include <SFML/Graphics.hpp>

class VirtualScreen : public sf::Drawable
{
public:
    void create(unsigned int width, unsigned int height, float pixel_size, sf::Color color);
    void setPixel(std::size_t x, std::size_t y, sf::Color color);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override ;

    sf::Vector2u m_screenSize;
    // virtual pixel size in real pixels
    float m_pixelSize;
    sf::VertexArray m_vertices;
};

#endif //SPNES_VIRTUALSCREEN_H
