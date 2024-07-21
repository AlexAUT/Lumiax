// NOLINTBEGIN
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#ifdef __cplusplus
extern "C"
#endif
    const char*
    __asan_default_options()
{
    return "detect_leaks=0";
}
// NOLINTEND

#include "box2d/b2_body.h"
#include "box2d/b2_world.h"
#include "debugRenderer.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "levelParser.hpp"
#include "levelRenderer.hpp"
#include "ship.hpp"
#include "shipRenderer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Joystick.hpp>
#include <iostream>
#include <print>
#include <ranges>
#include <vector>

#include <cmath>

int main()
{
    sf::RenderWindow window(sf::VideoMode{{1280, 720}},
                            "Lumiax",
                            sf::State::Windowed,
                            sf::ContextSettings{.antialiasingLevel = 8});
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
    {
        std::print(std::cerr, "Could not setup ImGui for SFML");
    }

    sf::Clock gameClock;

    ShipRenderer shipRenderer;
    DebugRenderer debugRenderer(window);
    debugRenderer.AppendFlags(b2Draw::e_shapeBit);
    debugRenderer.AppendFlags(b2Draw::e_centerOfMassBit);

    b2World world({0.f, 0.f});
    world.SetDebugDraw(&debugRenderer);


    std::vector ships = {createTriangleShip(world, sf::Color::Green, {1.f, 1.4f}, {5.f, 10.f}),
                         createTriangleShip(world, sf::Color::Red, {1.f, 1.5f}, {5.f, 5.f})};

    sf::Time fixedUpdateRate = sf::seconds(1.f / 60.f);
    sf::Time accumulatedTime{};
    sf::Time gameTime{};

    bool enableDebugDraw{false};

    auto levelResult = LevelParser::fromFile("../../data/levels/level01.json");
    if (!levelResult.has_value())
    {
        std::cout << "Failed to load level: " << levelResult.error();
        return 1;
    }
    Level& level = levelResult.value();

    level.registerCollision(world);
    LevelRenderer levelRenderer(level);

    while (window.isOpen())
    {
        auto deltaTime = gameClock.restart();

        while (std::optional<sf::Event> event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>(); keyEvent != nullptr)
            {
                if (keyEvent->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
            }
        }

        accumulatedTime += deltaTime;

        while (accumulatedTime >= fixedUpdateRate)
        {
            gameTime += deltaTime;

            ships[0].thruster(Ship::Direction::Up, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up));
            ships[0].thruster(Ship::Direction::Down, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down));
            ships[0].thruster(Ship::Direction::Left, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left));
            ships[0].thruster(Ship::Direction::Right, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right));

            ships[1].thruster(Ship::Direction::Up, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W));
            ships[1].thruster(Ship::Direction::Down, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S));
            ships[1].thruster(Ship::Direction::Left, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A));
            ships[1].thruster(Ship::Direction::Right, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D));

            std::cout << "Joystick connected: " << sf::Joystick::isConnected(0) << std::endl;

            float deadzone = 20.f;

            ships[1].thruster(Ship::Direction::Up,
                              sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::R) > (-100.f + deadzone));
            ships[1].thruster(Ship::Direction::Down,
                              sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Z) > (-100.f + deadzone));
            ships[1].thruster(Ship::Direction::Right, sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) > deadzone);
            ships[1].thruster(Ship::Direction::Left, sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) < -deadzone);

            for (auto& ship : ships)
                ship.update();

            accumulatedTime -= fixedUpdateRate;

            int32 velocityIterations = 8;
            int32 positionIterations = 3;

            level.updateAnimations(gameTime);

            world.Step(fixedUpdateRate.asSeconds(), velocityIterations, positionIterations);
        }


        ImGui::SFML::Update(window, deltaTime);

        window.clear();

        auto windowSize = static_cast<sf::Vector2f>(window.getSize());
        float aspect = windowSize.y / windowSize.x;

        float cameraWidth = 40.f;
        sf::Vector2f viewDim = sf::Vector2f(cameraWidth, cameraWidth * aspect);
        sf::Vector2f viewPosition = viewDim * 0.5f -
                                    (ships[0].position() + ships[0].centerOfMass().rotatedBy(ships[0].rotation()));

        sf::View view(sf::FloatRect(-viewPosition, viewDim));
        window.setView(view);

        ImGui::Begin("Debug");

        for (auto [index, ship] : ships | std::ranges::views::enumerate)
        {
            if (ImGui::CollapsingHeader(("Ship_" + std::to_string(index)).c_str()))
            {
                sf::Vector2f shipPos = ship.position();
                if (ImGui::SliderFloat2("Ship Position", &shipPos.x, 1.f, 1000.f))
                {
                    ship.position(shipPos);
                }
                b2Vec2 airResistance = ship.airResistance();
                ImGui::InputFloat2("Air resistance:", &airResistance.x);
                b2Vec2 linearAcceleration = ship.linearAccerleration();
                ImGui::InputFloat2("Linear acceleration:", &linearAcceleration.x);
                float a = ship.rotation().asDegrees();
                ImGui::InputFloat("Rotation (deg):", &a);
                float v = ship.body().GetLinearVelocity().Length();
                ImGui::InputFloat("Velocity :", &v);
            }
        }

        ImGui::Checkbox("Enable debug rendering", &enableDebugDraw);

        if (ImGui::CollapsingHeader("Joystick"))
        {
            for (int joystickIndex = 0; joystickIndex < 4; joystickIndex++)
            {
                if (sf::Joystick::isConnected(joystickIndex))
                {
                    ImGui::Text("Joystick: %d", joystickIndex);
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::X))
                    {
                        float x = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::X);
                        ImGui::InputFloat("X: ", &x);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::Y))
                    {
                        float y = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::Y);
                        ImGui::InputFloat("Y: ", &y);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::Z))
                    {
                        float z = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::Z);
                        ImGui::InputFloat("Z: ", &z);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::R))
                    {
                        float r = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::R);
                        ImGui::InputFloat("R: ", &r);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::U))
                    {
                        float u = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::U);
                        ImGui::InputFloat("U: ", &u);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::V))
                    {
                        float v = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::V);
                        ImGui::InputFloat("V: ", &v);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::PovX))
                    {
                        float v = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::PovX);
                        ImGui::InputFloat("PovX: ", &v);
                    }
                    if (sf::Joystick::hasAxis(joystickIndex, sf::Joystick::Axis::PovY))
                    {
                        float v = sf::Joystick::getAxisPosition(joystickIndex, sf::Joystick::Axis::PovY);
                        ImGui::InputFloat("PovY: ", &v);
                    }
                }
            }
        }

        ImGui::End();
        ImGui::SFML::Render(window);

        shipRenderer.reset();
        for (const auto& ship : ships)
        {
            shipRenderer.drawShip(ship);
        }

        levelRenderer.render(window);
        window.draw(shipRenderer.vertexArray());

        if (enableDebugDraw)
            world.DebugDraw();

        window.display();
    }

    ImGui::SFML::Shutdown();
}
