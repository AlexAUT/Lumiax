#include "levelParser.hpp"

#include "SFML/System/Vector2.hpp"
#include "nlohmann/json.hpp"

#include <fstream>

namespace LevelParser
{
std::expected<Level, std::string> fromFile(const std::filesystem::path& path)
{
    std::ifstream file(path);

    Level level;

    nlohmann::json levelFile;
    file >> levelFile;

    for (const auto& layer : levelFile["layers"])
    {
        std::string type = layer["type"];
        unsigned layerIndex = layer["id"].get<unsigned>() - 1;

        if (type == "tilelayer")
        {
            for (const auto& chunk : layer["chunks"])
            {
                level.addChunk(layerIndex,
                               {chunk["x"].get<int>(),
                                chunk["y"].get<int>(),
                                chunk["width"].get<int>(),
                                chunk["height"].get<int>(),
                                chunk["data"].get<std::vector<u32>>()});
            }
        }
        else if (type == "objectgroup")
        {
            for (const auto& object : layer["objects"])
            {
                // Check which type
                if (object.find("ellipse") != object.end())
                {
                    return std::unexpected("Ellipse not supported!");
                }
                else if (object.find("polyline") != object.end())
                {
                    sf::Vector2f pos{
                        object["x"].get<float>(),
                        object["y"].get<float>(),
                    };
                    Level::Animation animation;
                    for (const auto& p : object["polyline"])
                    {
                        animation.points.emplace_back(p["x"].get<float>() + pos.x, p["y"].get<float>() + pos.y);
                    }

                    if (object.contains("properties"))
                    {
                        for (const auto& prop : object["properties"])
                        {
                            if (prop["name"] == "duration")
                            {
                                if (prop["type"] != "float")
                                    return std::unexpected("Polyline property duration needs to be float");
                                animation.duration = prop["value"].get<float>();
                            }
                            else if (prop["name"] == "angularVelocity")
                            {
                                if (prop["type"] != "float")
                                    return std::unexpected("Polyline property angularVelocity needs to be float");
                                animation.angularVelocity = prop["value"].get<float>();
                            }
                            else
                            {
                                return std::unexpected("Unsupported property of polyline: " + prop["name"].get<std::string>());
                            }
                        }
                    }

                    level.addAnimation(layerIndex, object["id"].get<unsigned>(), std::move(animation));
                }
                else
                {
                    std::optional<unsigned> animationIndex;
                    if (object.contains("properties"))
                    {
                        for (const auto& prop : object["properties"])
                        {
                            if (prop["name"] == "animation")
                            {
                                if (prop["type"] != "object")
                                    return std::unexpected("Rect property animation needs to be of type object");
                                animationIndex = prop["value"].get<unsigned>();
                            }
                            else
                            {
                                return std::unexpected("Unsupported property of rect: " + prop["name"].get<std::string>());
                            }
                        }
                    }

                    level.addRect(layerIndex,
                                  object["id"].get<unsigned>(),
                                  Level::Rect{{object["x"].get<float>(), object["y"].get<float>()},
                                              {object["width"].get<float>(), object["height"].get<float>()}, sf::radians(0.f), animationIndex});
                }
            }
        }
    }


    return level;
}
} // namespace LevelParser
