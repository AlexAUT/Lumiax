#include "debugRenderer.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

void DebugRenderer::DrawPolygon(const b2Vec2*  /*vertices*/, int32  /*vertexCount*/, const b2Color&  /*color*/)
{
  throw std::runtime_error("Not implemented!");
}

void DebugRenderer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    sf::VertexArray vertexArray(sf::PrimitiveType::TriangleFan, vertexCount);
    sf::Vertex t{{}, sf::Color(color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f)};

    for (int i = 0; i < vertexCount; i++)
    {
      t.position = sf::Vector2f{vertices[i].x, vertices[i].y};
      vertexArray[i] = t;
    }

    mWindow.draw(vertexArray);
}

void DebugRenderer::DrawCircle(const b2Vec2&  /*center*/, float  /*radius*/, const b2Color&  /*color*/)
{
  throw std::runtime_error("Not implemented!");
}

void DebugRenderer::DrawSolidCircle(const b2Vec2&  /*center*/, float  /*radius*/, const b2Vec2&  /*axis*/, const b2Color&  /*color*/)
{
  throw std::runtime_error("Not implemented!");
}

void DebugRenderer::DrawSegment(const b2Vec2&  /*p1*/, const b2Vec2&  /*p2*/, const b2Color&  /*color*/)
{
  throw std::runtime_error("Not implemented!");
}

void DebugRenderer::DrawTransform(const b2Transform& xf)
{
  float size = 0.1f;
  sf::RectangleShape point({size, size});
  point.setPosition({xf.p.x - (size * 0.5f), xf.p.y - (size * 0.5f)});
  point.setFillColor(sf::Color::Red);
  mWindow.draw(point);
}

void DebugRenderer::DrawPoint(const b2Vec2&  /*p*/, float  /*size*/, const b2Color&  /*color*/)
{
  throw std::runtime_error("Not implemented!");
}
