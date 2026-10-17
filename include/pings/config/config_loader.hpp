#pragma once

#include <filesystem>

#include <pings/config/config_load_result.hpp>

namespace pings::config
{
    [[nodiscard]]
    ConfigLoadResult loadConfig(
        const std::filesystem::path &path);
}