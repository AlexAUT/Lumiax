#include "level.hpp"

#include "SFML/System/Time.hpp"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_world.h"

#include <stdexcept>

void Level::addChunk(unsigned layer, Chunk chunk)
{
    if (layer >= static_cast<unsigned>(mTiles.size()))
        mTiles.resize(layer + 1);

    mTiles[layer].emplace_back(std::move(chunk));
}

void Level::addRect(unsigned layer, unsigned id, Rect rect)
{
    if (layer >= static_cast<unsigned>(mRectLayers.size()))
        mRectLayers.resize(layer + 1);

    mRectLayers[layer].emplace_back(id, rect);
}

void Level::addAnimation(unsigned layer, unsigned id, Animation animation)
{
    if (layer >= static_cast<unsigned>(mAnimationLayers.size()))
        mAnimationLayers.resize(layer + 1);

    mAnimationLayers[layer].emplace_back(id, std::move(animation));
}

void Level::registerCollision(b2World& world)
{
    registerTileCollision(world);
    registerRectCollisions(world);
}

void Level::registerTileCollision(b2World& world)
{
    if (!mTileBodies.empty())
        throw std::runtime_error("register should only be called once");

    for (const auto& layer : mTiles)
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
                        b2Vec2 pos{static_cast<float>(chunk.x + x), static_cast<float>(chunk.y + y)};
                        b2Vec2 size{1.f, 1.f};

                        b2BodyDef bodyDef;
                        bodyDef.type = b2_staticBody;
                        bodyDef.position.Set(pos.x, pos.y);

                        b2Body* body = world.CreateBody(&bodyDef);

                        b2PolygonShape box;
                        box.SetAsBox(0.5f, 0.5f);

                        b2FixtureDef fixtureDef;
                        fixtureDef.shape = &box;

                        body->CreateFixture(&fixtureDef);

                        mTileBodies.push_back(body);
                    }
                }
            }
        }
    }
}
void Level::registerRectCollisions(b2World& world)
{
    if (!mRectBodyLayers.empty())
        throw std::runtime_error("register should only be called once");

    for (std::size_t layerIndex = 0; layerIndex < mRectLayers.size(); layerIndex++)
    {
        for (const auto& [id, rect] : mRectLayers[layerIndex])
        {
            b2BodyDef bodyDef;
            bodyDef.type = rect.animationIndex.has_value() ? b2_dynamicBody : b2_staticBody;
            bodyDef.position.Set(rect.position.x / 32.f, ((rect.position.y + (0.5f * rect.size.y)) / 32.f) - 0.5f);

            b2Body* boxBody = world.CreateBody(&bodyDef);

            b2PolygonShape box;
            box.SetAsBox(0.5f * rect.size.x / 32.f, 0.5f * rect.size.y / 32.f);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &box;
            fixtureDef.density = 0.f;

            boxBody->CreateFixture(&fixtureDef);

            if (mRectBodyLayers.size() <= layerIndex)
                mRectBodyLayers.resize(layerIndex + 1);

            mRectBodyLayers[layerIndex].emplace_back(id, boxBody);
        }
    }
}

void Level::updateAnimations(sf::Time& gameTime)
{
    for (std::size_t layerIndex = 0; layerIndex < mAnimationLayers.size(); layerIndex++)
    {
        for (auto& [id, rect] : mRectLayers[layerIndex])
        {
            if (!rect.animationIndex.has_value())
                continue;

            auto& animation = findAnimation(layerIndex, *rect.animationIndex);

            if (animation.points.size() != 2)
                throw std::runtime_error("Only animation paths with exactly 2 points are supported");


            // Hardcoded animation code
            float t = 2.f * (std::fmod(gameTime.asSeconds(), animation.duration) / animation.duration);
            if (t >= 1.f)
            {
                t = 2.f - t;
            }

            sf::Vector2f pos = {
                std::lerp(animation.points[0].x, animation.points[1].x, t) - (rect.size.x * 0.5f),
                std::lerp(animation.points[0].y, animation.points[1].y, t) - (rect.size.y * 0.5f),
            };

            rect.position = pos;

            b2Body& body = findRectBody(layerIndex, id);

            rect.rotation = sf::radians(body.GetAngle() + (animation.angularVelocity / 60.f));

            b2Vec2 physicsPos = {rect.position.x / 32.f, ((rect.position.y + (0.5f * rect.size.y)) / 32.f) - 0.5f};
            b2Vec2 diff = {
                60.f * (physicsPos.x - body.GetPosition().x),
                60.f * (physicsPos.y - body.GetPosition().y),
            };
            body.SetLinearVelocity(diff);
            body.SetAngularVelocity(animation.angularVelocity);
        }
    }
}

namespace level::priv
{
auto& findInLayerdContainer(auto& layeredIdContainer, unsigned layerIndex, unsigned id, std::string_view name = "")
{
    if (layerIndex >= layeredIdContainer.size())
        throw std::runtime_error("Animation layer not present");

    auto found = std::ranges::find_if(layeredIdContainer[layerIndex],
                                      [id](const auto& idWrapper) { return idWrapper.id == id; });

    if (found == layeredIdContainer[layerIndex].end())
        throw std::runtime_error(std::format("{} in layer {} with id {} could not be found!", name, layerIndex, id));

    return found->value;
}
} // namespace level::priv

Level::Animation& Level::findAnimation(unsigned layerIndex, unsigned animationId)
{
    return level::priv::findInLayerdContainer(mAnimationLayers, layerIndex, animationId, "Animation");
}

b2Body& Level::findRectBody(unsigned layerIndex, unsigned rectId)
{
    return *level::priv::findInLayerdContainer(mRectBodyLayers, layerIndex, rectId, "Rect Physics Body");
}
