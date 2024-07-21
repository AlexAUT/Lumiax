#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

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

  sf::Texture mTileset;
  std::vector<sf::RectangleShape> mRects;
};
