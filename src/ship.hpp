#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include "box2d/b2_math.h"

namespace sf
{
class Time;
}

class b2World;
class b2Body;

class Ship
{
public:
    enum class Direction
    {
        Left,
        Up,
        Right,
        Down
    };

    Ship(sf::Color color, b2Body* body);

    sf::Vector2f position() const;
    void position(sf::Vector2f newPos);

    sf::Vector2f centerOfMass() const;

    sf::Color color() const { return mColor; }
    void color(sf::Color newColor) { mColor = newColor; }

    sf::Angle rotation() const;
    // void rotation(sf::Angle newRotation) { mRotation = newRotation; }

    void thruster(Direction dir, bool state);
    bool thruster(Direction dir) const;

    void update();

    const b2Body& body() const { return *mBody; }

    b2Vec2 airResistance() const;
    b2Vec2 linearAccerleration() const;
    float angularAccerleration() const;
private:

    sf::Color mColor{sf::Color::Green};
    b2Body* mBody{};

    float mLinearThrusterAcceleration{5.f};
    float mAngularThrusterAccerlaration{1.f};

    std::array<bool, 4> mThrusterState{};
};

Ship createTriangleShip(b2World& world, sf::Color color, sf::Vector2f size, sf::Vector2f position = {}, sf::Angle angle = {});
