#include <cstddef>
#include <string>

#include <pings/config/config_validation.hpp>
#include <pings/config/config_mapping.hpp>

#include <pings/ping_options.hpp>

namespace pings::config
{
    namespace
    {
        std::string makeTimeoutRangeMessage(std::chrono::milliseconds min, std::chrono::milliseconds max)
        {
            return "Value must be between " +
                   std::to_string(min.count()) +
                   " ms and " +
                   std::to_string(max.count()) +
                   " ms";
        }

        std::string makePayloadSizeMessage()
        {
            return "Value must not exceed " +
                   std::to_string(MaxPayloadSize) +
                   " bytes";
        }

        void validatePingOptions(
            const PingOptions &options,
            std::string_view prefix,
            ValidationResult &result)
        {
            if (options.replyTimeout < MinReplyTimeout ||
                options.replyTimeout > MaxReplyTimeout)
            {
                result.errors.push_back({std::string(prefix) + ".replyTimeout",
                                         makeTimeoutRangeMessage(
                                             MinReplyTimeout,
                                             MaxReplyTimeout)});
            }

            if (options.resolveTimeout < MinResolveTimeout ||
                options.resolveTimeout > MaxResolveTimeout)
            {
                result.errors.push_back({std::string(prefix) + ".resolveTimeout",
                                         makeTimeoutRangeMessage(
                                             MinResolveTimeout,
                                             MaxResolveTimeout)});
            }

            if (options.payloadSize > MaxPayloadSize)
            {
                result.errors.push_back({std::string(prefix) + ".payloadSize",
                                         makePayloadSizeMessage()});
            }

            switch (options.addressFamily)
            {
            case AddressFamily::Any:
            case AddressFamily::IPv4:
            case AddressFamily::IPv6:
                break;

            default:
                result.errors.push_back({std::string(prefix) + ".addressFamily",
                                         "Value must be Any, IPv4 or IPv6"});
                break;
            }
        }
    }

    ValidationResult validate(const AppConfig &config)
    {
        ValidationResult result;

        validatePingOptions(
            config.defaults,
            "defaults",
            result);

        for (std::size_t i = 0; i < config.hosts.size(); ++i)
        {
            const auto &host = config.hosts[i];

            const auto prefix = "hosts[" + std::to_string(i) + "]";

            if (host.name.empty())
            {
                result.errors.push_back({prefix + ".name",
                                         "Name must not be empty"});
            }

            if (host.host.empty())
            {
                result.errors.push_back({prefix + ".host",
                                         "Host must not be empty"});
            }

            const auto options =
                makePingOptions(
                    config.defaults,
                    host.ping);

            validatePingOptions(
                options,
                prefix + ".ping",
                result);
        }

        return result;
    }
}