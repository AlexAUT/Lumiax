#include "ship.hpp"

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_world.h"

#include <SFML/System/Time.hpp>
#include <algorithm>

Ship::Ship(sf::Color color, b2Body* body) : mColor{color}, mBody{body}
{
}

sf::Vector2f Ship::position() const
{
    auto p = mBody->GetPosition();
    return {p.x, p.y};
}

void Ship::position(sf::Vector2f newPos)
{
    mBody->SetTransform(b2Vec2{newPos.x, newPos.y}, mBody->GetAngle());
    mBody->SetAwake(true);
}

sf::Vector2f Ship::centerOfMass() const
{
    auto massData = mBody->GetMassData();
    return {massData.center.x, massData.center.y};
}

sf::Angle Ship::rotation() const
{
    return sf::radians(mBody->GetAngle());
}

void Ship::thruster(Direction dir, bool newState)
{
    mThrusterState[static_cast<std::size_t>(dir)] = newState;
}

bool Ship::thruster(Direction dir) const
{
    return mThrusterState[static_cast<std::size_t>(dir)];
}


void Ship::update()
{
    b2Vec2 force = linearAccerleration() + airResistance();

    // sf::Vector2f direction{std::sin(rotation().asRadians()), std::cos(rotation().asRadians())};
    // b2Vec2 force{direction.x * mForwardThrust * dt, -direction.y * mForwardThrust * dt};
    mBody->ApplyForceToCenter(force, true);

    // mBody->ApplyForceToCenter(force, true);
    mBody->ApplyTorque(angularAccerleration(), true);
}

b2Vec2 Ship::airResistance() const
{
    // Calculate drag

    float airDensity = 1.2f;
    float crossSection = 1.0f; //TODO Compute based on rotation and velocity direction
    float dragCoefficient = 0.03f;

    b2Vec2 linearVelocity = mBody->GetLinearVelocity();

    float vSquared = mBody->GetLinearVelocity().LengthSquared();

    if (vSquared < 0.00001f)
        return {};

    float dragForce = 0.5f * airDensity * vSquared * dragCoefficient * crossSection;
    // we need the angle of the body's current velocity to know which angle we should set the drag force

    float v = mBody->GetLinearVelocity().Length();
    b2Vec2 linearVelocityNorm{
        linearVelocity.x / v,
        linearVelocity.y / v,
    };

    return {
        -linearVelocityNorm.x * dragForce,
        -linearVelocityNorm.y * dragForce,
    };
}

b2Vec2 Ship::linearAccerleration() const
{
    float forwardThrust = (thruster(Direction::Up) ? mLinearThrusterAcceleration : 0.f) +
                          (thruster(Direction::Down) ? -mLinearThrusterAcceleration : 0.f);


    if (forwardThrust == 0.f)
        return {};


    float angle = rotation().asRadians();
    // b2Vec2 force{direction.x * mForwardThrust * dt, -direction.y * mForwardThrust * dt};

    return {std::sin(angle) * forwardThrust, -std::cos(angle) * forwardThrust};
}

float Ship::angularAccerleration() const
{
    return (thruster(Direction::Left) ? -mAngularThrusterAccerlaration : 0.f) +
           (thruster(Direction::Right) ? mAngularThrusterAccerlaration : 0.f);
}

Ship createTriangleShip(b2World& world, sf::Color color, sf::Vector2f size, sf::Vector2f position, sf::Angle angle)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    bodyDef.angle = angle.asRadians();
    bodyDef.linearDamping = 1.f;
    bodyDef.angularDamping = 2.f;

    b2Body* body = world.CreateBody(&bodyDef);
    auto sizeHalf = size * 0.5f;

    b2PolygonShape dynamicBox;
    std::array<b2Vec2, 3> vertices = {
        b2Vec2{0.f, -sizeHalf.y},
        b2Vec2{sizeHalf.x, sizeHalf.y},
        b2Vec2{-sizeHalf.x, sizeHalf.y},
    };
    dynamicBox.Set(vertices.data(), 3);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 0.6f;
    fixtureDef.friction = 0.8f;
    fixtureDef.restitution = 0.01f;

    body->CreateFixture(&fixtureDef);

    return {color, body};
}
