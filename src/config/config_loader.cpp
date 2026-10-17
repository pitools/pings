#include <filesystem>
#include <yaml-cpp/yaml.h>

#include <pings/config/config_loader.hpp>

namespace pings::config
{

    namespace
    {
        AddressFamily readAddressFamily(const YAML::Node &node)
        {
            const auto value = node.as<std::string>();

            if (value == "any")
                return AddressFamily::Any;

            if (value == "ipv4")
                return AddressFamily::IPv4;

            if (value == "ipv6")
                return AddressFamily::IPv6;

            throw YAML::RepresentationException(
                node.Mark(),
                "address_family must be 'any', 'ipv4' or 'ipv6'");
        }

        void readDefaults(
            const YAML::Node &node,
            AppConfig &config)
        {
            if (!node)
            {
                return;
            }

            if (node["reply_timeout_ms"])
            {
                config.defaults.replyTimeout =
                    std::chrono::milliseconds{
                        node["reply_timeout_ms"].as<std::int64_t>()};
            }

            if (node["resolve_timeout_ms"])
            {
                config.defaults.resolveTimeout =
                    std::chrono::milliseconds{
                        node["resolve_timeout_ms"].as<std::int64_t>()};
            }

            if (node["payload_size"])
            {
                config.defaults.payloadSize =
                    node["payload_size"].as<std::size_t>();
            }

            if (node["address_family"])
            {
                config.defaults.addressFamily =
                    readAddressFamily(node["address_family"]);
            }
        }

        PingOptionsOverride readPingOverride(
            const YAML::Node &node)
        {
            PingOptionsOverride result;

            if (!node)
            {
                return result;
            }

            if (node["reply_timeout_ms"])
            {
                result.replyTimeout =
                    std::chrono::milliseconds{
                        node["reply_timeout_ms"].as<std::int64_t>()};
            }

            if (node["resolve_timeout_ms"])
            {
                result.resolveTimeout =
                    std::chrono::milliseconds{
                        node["resolve_timeout_ms"].as<std::int64_t>()};
            }

            if (node["payload_size"])
            {
                result.payloadSize =
                    node["payload_size"].as<std::size_t>();
            }

            if (node["address_family"])
            {
                result.addressFamily =
                    readAddressFamily(node["address_family"]);
            }

            return result;
        }

        HostConfig readHostConfig(
            const YAML::Node &node)
        {
            HostConfig host;

            if (node["name"])
            {
                host.name =
                    node["name"].as<std::string>();
            }

            if (node["host"])
            {
                host.host =
                    node["host"].as<std::string>();
            }

            if (node["enabled"])
            {
                host.enabled =
                    node["enabled"].as<bool>();
            }

            host.ping =
                readPingOverride(node["ping"]);

            return host;
        }

        std::size_t yamlPositionToSize(int value)
        {
            return value >= 0
                       ? static_cast<std::size_t>(value + 1)
                       : 0;
        }
    }

    ConfigLoadResult loadConfig(
        const std::filesystem::path &path)
    {
        ConfigLoadResult result;

        try
        {
            const YAML::Node root = YAML::LoadFile(path.string());

            // The YAML::Node::IsMap() function is an inline member function that checks if the current YAML node is of type Map.
            // It returns a boolean value indicating whether the node's type matches NodeType::Map.
            if (!root.IsMap())
            {
                result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                         "Root YAML node must be a map"});

                return result;
            }

            const YAML::Node defaults = root["defaults"];

            if (defaults && !defaults.IsMap())
            {
                result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                         "'defaults' must be a map",
                                         yamlPositionToSize(defaults.Mark().line),
                                         yamlPositionToSize(defaults.Mark().column)});

                return result;
            }

            const YAML::Node hosts = root["hosts"];

            if (hosts && !hosts.IsSequence())
            {
                result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                         "'hosts' must be a sequence",
                                         yamlPositionToSize(hosts.Mark().line),
                                         yamlPositionToSize(hosts.Mark().column)});

                return result;
            }

            AppConfig config;

            readDefaults(root["defaults"], config);

            if (hosts)
            {
                for (const auto &hostNode : hosts)
                {
                    if (!hostNode.IsMap())
                    {
                        result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                                 "Each item in 'hosts' must be a map",
                                                 yamlPositionToSize(hostNode.Mark().line),
                                                 yamlPositionToSize(hostNode.Mark().column)});

                        return result;
                    }

                    const YAML::Node ping = hostNode["ping"];

                    if (ping && !ping.IsMap())
                    {
                        result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                                 "'ping' must be a map",
                                                 yamlPositionToSize(ping.Mark().line),
                                                 yamlPositionToSize(ping.Mark().column)});

                        return result;
                    }

                    config.hosts.push_back(readHostConfig(hostNode));
                }
            }

            result.config = std::move(config);
        }
        catch (const YAML::BadFile &e)
        {
            result.errors.push_back({ConfigLoadErrorCode::FileNotFound,
                                     e.what()});
        }
        catch (const YAML::ParserException &e)
        {
            result.errors.push_back({ConfigLoadErrorCode::ParseError,
                                     e.what(),
                                     yamlPositionToSize(e.mark.line),
                                     yamlPositionToSize(e.mark.column)});
        }
        catch (const YAML::BadConversion &e)
        {
            result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                     e.what(),
                                     yamlPositionToSize(e.mark.line),
                                     yamlPositionToSize(e.mark.column)});
        }
        catch (const YAML::RepresentationException &e)
        {
            result.errors.push_back({ConfigLoadErrorCode::TypeError,
                                     e.what(),
                                     yamlPositionToSize(e.mark.line),
                                     yamlPositionToSize(e.mark.column)});
        }

        return result;
    }
}