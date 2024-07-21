#include "levelParser.hpp"

#include "SFML/Graphics/Image.hpp"
#include "SFML/System/Vector2.hpp"
#include "nlohmann/json.hpp"

#include <fstream>

namespace LevelParser
{
std::optional<std::string> parseLayers(Level& level, nlohmann::json& levelFile);
std::optional<std::string> parseTilesets(Level& level, nlohmann::json& levelFile, const std::filesystem::path& levelPath);

std::expected<Level, std::string> fromFile(const std::filesystem::path& path)
{
    std::ifstream file(path);

    if (!file.is_open())
        return std::unexpected(std::format("Could not open level file: {}", path.c_str()));

    nlohmann::json levelFile;
    file >> levelFile;

    Level level;

    if (auto error = parseLayers(level, levelFile); error.has_value())
        return std::unexpected(*error);

    if (auto error = parseTilesets(level, levelFile, path); error.has_value())
        return std::unexpected(*error);

    return level;
}

std::optional<std::string> parseLayers(Level& level, nlohmann::json& levelFile)
{
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
                    return "Ellipse not supported!";
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
                                    return "Polyline property duration needs to be float";
                                animation.duration = prop["value"].get<float>();
                            }
                            else if (prop["name"] == "angularVelocity")
                            {
                                if (prop["type"] != "float")
                                    return "Polyline property angularVelocity needs to be float";
                                animation.angularVelocity = prop["value"].get<float>();
                            }
                            else
                            {
                                return "Unsupported property of polyline: " + prop["name"].get<std::string>();
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
                                    return "Rect property animation needs to be of type object";
                                animationIndex = prop["value"].get<unsigned>();
                            }
                            else
                            {
                                return "Unsupported property of rect: " + prop["name"].get<std::string>();
                            }
                        }
                    }

                    level.addRect(layerIndex,
                                  object["id"].get<unsigned>(),
                                  Level::Rect{{object["x"].get<float>(), object["y"].get<float>()},
                                              {object["width"].get<float>(), object["height"].get<float>()},
                                              sf::radians(0.f),
                                              animationIndex});
                }
            }
        }
    }

    return {};
}

std::optional<std::string> parseTilesets(Level& level, nlohmann::json& levelFile, const std::filesystem::path& levelPath)
{
    auto basePath = levelPath.parent_path();
    for (const auto& tileset : levelFile["tilesets"])
    {
        auto tilesetPath = basePath / tileset["source"];
        std::ifstream tilesetFile(tilesetPath);

        if (!tilesetFile.is_open())
            return std::format("Could not open tileset file: {}", tilesetPath.c_str());

        auto firstgid = tileset["firstgid"];

        nlohmann::json tilesetDesc;
        tilesetFile >> tilesetDesc;

        auto imagePath = basePath / tilesetDesc["image"];
        auto imageLoadResult = sf::Image::loadFromFile(imagePath);
        if (!imageLoadResult.has_value())
            return std::format("Could not read tile image: {}", imagePath.c_str());

        level.addTileset(Level::Tileset{
            std::move(*imageLoadResult),
            tileset["firstgid"].get<unsigned>(),
            {tilesetDesc["tilewidth"].get<unsigned>(), tilesetDesc["tileheight"].get<unsigned>()},
            tilesetDesc["columns"].get<unsigned>(),
        });
    }

    return {};
}

} // namespace LevelParser
