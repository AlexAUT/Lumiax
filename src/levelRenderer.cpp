#include "levelRenderer.hpp"

#include "level.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

LevelRenderer::LevelRenderer(const Level& level) : mLevel{level}
{
    const auto& tilesets = level.getTilesets();
    for (const auto& tileset : tilesets)
    {
        auto tex = mTilesetTextures.emplace_back(*sf::Texture::loadFromImage(tileset.image));
        tex.setSmooth(false);
        tex.setRepeated(false);
        if (!tex.generateMipmap())
        {
            std::cout << "Could not generate mip maps for tilesets" << std::endl;
        }
    }

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

                        u32 rawTileData = chunk.data[linearIndex];

                        u32 globalTileId = rawTileData & (~(flippedHorizontallyFlag | flippedVerticallyFlag |
                                                            flippedDiagonallyFlag | rotatedHexagonal120Flag));

                        std::size_t tilesetIndex = 0;
                        while ((tilesetIndex + 1) < tilesets.size() && tilesets[tilesetIndex + 1].firstGid < globalTileId)
                        {
                            tilesetIndex++;
                        }

                        const auto& tileset = tilesets[tilesetIndex];

                        u32 localTileId = globalTileId - tileset.firstGid;
                        u32 col = localTileId % tileset.columns;
                        u32 row = localTileId / tileset.columns;

                        sf::IntRect texRect{{static_cast<int>(col * tileset.tileDim.x),
                                             static_cast<int>(row * tileset.tileDim.y)},
                                            static_cast<sf::Vector2i>(tileset.tileDim)};

                        // Move origin to center to allow for easier tile rotations
                        shape.setOrigin(shape.getSize() * 0.5f);
                        shape.move(shape.getSize() * 0.5f);
                        if (rawTileData & flippedDiagonallyFlag)
                        {
                            shape.rotate(sf::degrees(90.f));
                            shape.scale({-1.f, 1.f});
                        }
                        if (rawTileData & flippedHorizontallyFlag)
                        {
                            shape.scale({-1.f, 1.f});
                        }
                        if (rawTileData & flippedVerticallyFlag)
                        {
                            shape.scale({1.f, -1.f});
                        }
                        if (rawTileData & rotatedHexagonal120Flag)
                        {
                            throw std::runtime_error("Rotated hexagon 120 tile rendering not implemented!");
                        }

                        shape.setTexture(&mTilesetTextures[tilesetIndex]);
                        shape.setTextureRect(texRect);
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
