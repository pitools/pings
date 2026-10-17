#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <pings/config/app_config.hpp>

namespace pings::config
{
    enum class ConfigLoadErrorCode
    {
        FileNotFound,
        AccessDenied,
        IoError,
        ParseError,
        TypeError
    };

    constexpr std::string_view toString(
        ConfigLoadErrorCode code) noexcept
    {
        switch (code)
        {
        case ConfigLoadErrorCode::FileNotFound:
            return "file_not_found";

        case ConfigLoadErrorCode::AccessDenied:
            return "access_denied";

        case ConfigLoadErrorCode::IoError:
            return "io_error";

        case ConfigLoadErrorCode::ParseError:
            return "parse_error";

        case ConfigLoadErrorCode::TypeError:
            return "type_error";
        }

        return "unknown";
    }

    struct ConfigLoadError
    {
        ConfigLoadErrorCode code{ConfigLoadErrorCode::IoError};

        std::string message;

        std::size_t line{};
        std::size_t column{};
    };

    struct ConfigLoadResult
    {
        std::optional<AppConfig> config;
        std::vector<ConfigLoadError> errors;

        [[nodiscard]]
        bool success() const noexcept
        {
            return config.has_value() && errors.empty();
        }
    };
}