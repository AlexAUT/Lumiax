#pragma once

#include <SFML/Graphics/VertexArray.hpp>

class Ship;

class ShipRenderer
{
public:
    void reset();
    void drawShip(const Ship& ship);

    const sf::VertexArray& vertexArray() const { return mVertices; }

private:
    sf::VertexArray mVertices;
};