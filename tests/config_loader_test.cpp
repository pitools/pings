#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include <pings/config/config_loader.hpp>
#include <pings/config/config_mapping.hpp>
#include <pings/config/config_validation.hpp>

namespace
{
    int checks = 0;
    int failures = 0;

    void expect(bool condition, std::string_view name)
    {
        ++checks;

        if (!condition)
        {
            ++failures;
            std::cerr << "FAIL: " << name << '\n';
        }
    }
}

int main()
{
    using pings::AddressFamily;
    using namespace pings::config;

    const auto families = loadConfig("families.yml");

    expect(families.success(), "valid family settings load");

    if (families.success())
    {
        const auto &config = *families.config;

        expect(
            config.defaults.addressFamily == AddressFamily::IPv4,
            "global IPv4 setting");

        expect(
            validate(config).valid(),
            "loaded configuration passes validation");

        const auto targets = makePingTargets(config);

        expect(targets.size() == 3, "three targets");

        if (targets.size() == 3)
        {
            expect(
                targets[0].options.addressFamily ==
                    AddressFamily::IPv4,
                "missing host setting inherits IPv4");

            expect(
                targets[1].options.addressFamily ==
                    AddressFamily::IPv6,
                "host overrides family with IPv6");

            expect(
                targets[2].options.addressFamily ==
                    AddressFamily::Any,
                "explicit Any overrides global IPv4");
        }
    }

    const auto missing = loadConfig("family_missing.yml");

    expect(missing.success(), "omitted family setting loads");

    if (missing.success())
    {
        expect(
            missing.config->defaults.addressFamily ==
                AddressFamily::Any,
            "omitted global family defaults to Any");
    }

    const auto checkInvalid = [](const std::filesystem::path &path)
    {
        const auto result = loadConfig(path);

        expect(
            !result.success() && !result.config,
            "invalid family does not produce configuration");

        expect(
            result.errors.size() == 1,
            "invalid family produces one error");

        if (result.errors.size() == 1)
        {
            const auto &error = result.errors.front();

            expect(
                error.code == ConfigLoadErrorCode::TypeError,
                "invalid family error category");

            expect(
                error.line > 0 && error.column > 0,
                "error includes source position");

            expect(
                error.message.find("address_family") !=
                    std::string::npos,
                "error identifies the setting");
        }
    };

    checkInvalid("family_invalid_default.yml");
    checkInvalid("family_invalid_host.yml");

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}