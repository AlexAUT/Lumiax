#include "shipRenderer.hpp"

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_shape.h"
#include "box2d/b2_polygon_shape.h"
#include "ship.hpp"
#include <stdexcept>

void ShipRenderer::reset()
{
    mVertices.clear();
}

void ShipRenderer::drawShip(const Ship& ship)
{
    sf::Transform transform;
    transform.translate(ship.position());
    transform.rotate(ship.rotation());

    mVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    auto oldSize = mVertices.getVertexCount();

    const auto& body = ship.body();
    const auto* shape = body.GetFixtureList()[0].GetShape();

    if (shape->GetType() == b2Shape::Type::e_polygon)
    {
        const auto* polygon = static_cast<const b2PolygonShape*>(shape);

        mVertices.resize(oldSize + polygon->m_count);

        for (int32 i = 0; i < polygon->m_count; i++)
        {
            b2Vec2 pos = body.GetWorldPoint(polygon->m_vertices[i]);
            mVertices[oldSize + i] = sf::Vertex{{pos.x, pos.y}, ship.color()};
        }
    }
    else
    {
        throw std::runtime_error("Shape type rendering not implemented!");
    }
}
