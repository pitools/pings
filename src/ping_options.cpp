#include <pings/ping_options.hpp>

namespace pings
{

    bool isValid(const PingOptions &options) noexcept
    {
        if (options.replyTimeout < MinReplyTimeout || options.replyTimeout > MaxReplyTimeout)
            return false;

        if (options.resolveTimeout < MinResolveTimeout || options.resolveTimeout > MaxResolveTimeout)
            return false;

        if (options.payloadSize > MaxPayloadSize)
            return false;

        switch (options.addressFamily)
        {
        case AddressFamily::Any:
        case AddressFamily::IPv4:
        case AddressFamily::IPv6:
            break;

        default:
            return false;
        }

        return true;
    }
}
