#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace sf
{
class RenderWindow;
}

class Level;

class LevelRenderer
{
public:
    LevelRenderer(const Level& level);
    void render(sf::RenderWindow& window);

private:
    void drawTiles(sf::RenderWindow& window);
    void drawRects(sf::RenderWindow& window);

    const Level& mLevel;

    std::vector<sf::Texture> mTilesetTextures;
    std::vector<sf::RectangleShape> mRects;
};
