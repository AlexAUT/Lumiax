#pragma once

#include <expected>
#include <filesystem>

#include "level.hpp"

namespace LevelParser
{
std::expected<Level, std::string> fromFile(const std::filesystem::path& path);
}
