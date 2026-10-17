#include <cstdlib>
#include <iostream>
#include <string_view>

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

    pings::PingOptions defaults{};
    defaults.addressFamily = AddressFamily::IPv4;

    PingOptionsOverride overrides{};

    expect(
        makePingOptions(defaults, overrides).addressFamily ==
            AddressFamily::IPv4,
        "missing override inherits IPv4");

    overrides.addressFamily = AddressFamily::IPv6;

    expect(
        makePingOptions(defaults, overrides).addressFamily ==
            AddressFamily::IPv6,
        "IPv6 overrides IPv4");

    overrides.addressFamily = AddressFamily::Any;

    expect(
        makePingOptions(defaults, overrides).addressFamily ==
            AddressFamily::Any,
        "explicit Any overrides IPv4");

    overrides.addressFamily.reset();

    expect(
        makePingOptions(defaults, overrides).addressFamily ==
            AddressFamily::IPv4,
        "reset restores inheritance");

    expect(
        defaults.addressFamily == AddressFamily::IPv4,
        "mapping does not modify defaults");

    AppConfig config{};
    config.defaults = defaults;

    HostConfig host{};
    host.name = "loopback";
    host.host = "::1";
    host.ping.addressFamily = AddressFamily::IPv6;
    config.hosts.push_back(host);

    const auto targets = makePingTargets(config);

    expect(
        targets.size() == 1 &&
            targets[0].options.addressFamily == AddressFamily::IPv6,
        "target receives effective family");

    expect(validate(config).valid(), "valid configuration");

    config.defaults.addressFamily =
        static_cast<AddressFamily>(999);

    const auto invalidDefaults = validate(config);

    expect(
        !invalidDefaults.valid() &&
            invalidDefaults.errors[0].field ==
                "defaults.addressFamily",
        "invalid default family is reported");

    config.defaults.addressFamily = AddressFamily::IPv4;
    config.hosts[0].ping.addressFamily =
        static_cast<AddressFamily>(999);

    const auto invalidHost = validate(config);

    expect(
        !invalidHost.valid() &&
            invalidHost.errors[0].field ==
                "hosts[0].ping.addressFamily",
        "invalid host family is reported");

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}