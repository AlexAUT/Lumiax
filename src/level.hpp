#pragma once

#include "SFML/System/Vector2.hpp"
#include "types.hpp"

#include <filesystem>
#include <vector>

class b2World;
class b2Body;

namespace sf
{
class Time;
}

class Level
{
public:
    struct Chunk
    {
        int x{};
        int y{};
        int width{};
        int height{};
        std::vector<u32> data;
    };
    struct Rect
    {
        sf::Vector2f position{};
        sf::Vector2f size{};
        sf::Angle rotation{sf::radians(0.f)};
        std::optional<unsigned> animationIndex{};
    };

    struct Animation
    {
        std::vector<sf::Vector2f> points;
        float duration{};
        float angularVelocity{};
    };

    template <typename Val>
    struct IdWrapper
    {
        unsigned id;
        Val value;
    };

    void addChunk(unsigned layer, Chunk chunk);
    void addRect(unsigned layer, unsigned id, Rect rect);
    void addAnimation(unsigned layer, unsigned id, Animation animation);

    std::vector<std::vector<IdWrapper<Rect>>>& getRects() { return mRectLayers; }

    const std::vector<std::vector<Chunk>>& getTileLayers() const { return mTiles; }
    const std::vector<std::vector<IdWrapper<Rect>>>& getRectLayers() const { return mRectLayers; }
    const std::vector<std::vector<IdWrapper<Animation>>>& getPolylineLayers() const { return mAnimationLayers; }

    const std::filesystem::path& tilesetPath() const { return mTilesetPath; }

    void registerCollision(b2World& world);

    void updateAnimations(sf::Time& gameTime);

private:
    void registerTileCollision(b2World& world);
    void registerRectCollisions(b2World& world);

    Animation& findAnimation(unsigned layerIndex, unsigned animationId);
    b2Body& findRectBody(unsigned layerIndex, unsigned rectId);

    std::vector<std::vector<Chunk>> mTiles;
    std::vector<std::vector<IdWrapper<Rect>>> mRectLayers;
    std::vector<std::vector<IdWrapper<Animation>>> mAnimationLayers;

    std::vector<b2Body*> mTileBodies;
    std::vector<std::vector<IdWrapper<b2Body*>>> mRectBodyLayers;
    std::filesystem::path mTilesetPath{"../../data/tilesets/glozzom24-32x.png"};
};
