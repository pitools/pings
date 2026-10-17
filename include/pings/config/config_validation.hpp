#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <pings/config/app_config.hpp>

namespace pings::config
{
    struct ValidationError
    {
        std::string field;
        std::string message;
    };

    struct ValidationResult
    {
        std::vector<ValidationError> errors;

        [[nodiscard]]
        bool valid() const noexcept
        {
            return errors.empty();
        }
    };

    [[nodiscard]]
    ValidationResult validate(
        const AppConfig &config);
}