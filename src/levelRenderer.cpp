#include "levelRenderer.hpp"

#include "level.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

LevelRenderer::LevelRenderer(const Level& level) : mLevel{level}, mTileset(sf::Texture::loadFromFile(level.tilesetPath()).value())
{
    mTileset.setSmooth(true);
    for (const auto& layer : level.getTileLayers())
    {
        for (const auto& chunk : layer)
        {
            for (int x = 0; x < chunk.width; x++)
            {
                for (int y = 0; y < chunk.height; y++)
                {
                    int linearIndex = x + (y * chunk.width);

                    if (chunk.data[linearIndex] != 0)
                    {
                        sf::Vector2f pos{static_cast<float>(chunk.x + x) - 0.5f, static_cast<float>(chunk.y + y) - 0.5f};
                        sf::Vector2f size{1.f, 1.f};
                        sf::RectangleShape shape(size);
                        shape.setPosition(pos);
                        shape.setFillColor(sf::Color::White);

                        const unsigned flippedHorizontallyFlag = 0x80000000;
                        const unsigned flippedVerticallyFlag = 0x40000000;
                        const unsigned flippedDiagonallyFlag = 0x20000000;
                        const unsigned rotatedHexagonal120Flag = 0x10000000;

                        u32 globalTileId = chunk.data[linearIndex] & (~(flippedHorizontallyFlag | flippedVerticallyFlag |
                                                                        flippedDiagonallyFlag | rotatedHexagonal120Flag));

                        shape.setTexture(&mTileset);
                        shape.setTextureRect(sf::IntRect({static_cast<int>(32 * (globalTileId - 1)) + 2, 2}, {28, 28}));
                        mRects.push_back(shape);
                    }
                }
            }
        }
    }
}

void LevelRenderer::render(sf::RenderWindow& window)
{
    drawTiles(window);
    drawRects(window);
}

void LevelRenderer::drawTiles(sf::RenderWindow& window)
{
    for (const auto& rect : mRects)
    {
        window.draw(rect);
    }
}

void LevelRenderer::drawRects(sf::RenderWindow& window)
{
    for (const auto& layer : mLevel.getRectLayers())
    {
        for (const auto& object : layer)
        {
            const auto& rect = object.value;

            sf::RectangleShape shape(rect.size / 32.f);
            shape.setOrigin((rect.size * 0.5f) / 32.f);
            shape.setPosition((rect.position / 32.f) - sf::Vector2f{0.5f, 0.5f} + shape.getOrigin());
            shape.setFillColor(sf::Color::Blue);
            shape.setRotation(rect.rotation);
            window.draw(shape);
        }
    }
}
