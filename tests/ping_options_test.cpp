#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <pings/ping_options.hpp>

namespace
{
    int failures = 0;
    int checks = 0;

    void expectValidity(
        std::string_view name,
        const pings::PingOptions &options,
        bool expected)
    {
        ++checks;

        const bool actual = pings::isValid(options);

        if (actual != expected)
        {
            ++failures;

            std::cerr
                << "FAIL: " << name
                << ", expected valid = " << expected
                << ", actual" << actual << '\n';
        }
    }
}

int main()
{
    using namespace std::chrono_literals;

    expectValidity("defaults", pings::PingOptions{}, true);

    // replyTimeout
    {
        pings::PingOptions options{};

        options.replyTimeout = pings::MinReplyTimeout;
        expectValidity("reply timeout: minimum", options, true);

        options.replyTimeout = pings::MaxReplyTimeout;
        expectValidity("reply timeout: maximum", options, true);

        options.replyTimeout = pings::MinReplyTimeout - 1ms;
        expectValidity("reply timeout: below minimum", options, false);

        options.replyTimeout = pings::MaxReplyTimeout + 1ms;
        expectValidity("reply timeout: above maximum", options, false);

        options.replyTimeout = -1ms;
        expectValidity("reply timeout: negative", options, false);
    }

    // resolveTimeout
    {
        pings::PingOptions options{};

        options.resolveTimeout = pings::MinResolveTimeout;
        expectValidity("resolve timeout: minimum", options, true);

        options.resolveTimeout = pings::MaxResolveTimeout;
        expectValidity("resolve timeout: maximum", options, true);

        options.resolveTimeout = pings::MinResolveTimeout - 1ms;
        expectValidity("resolve timeout: below minimum", options, false);

        options.resolveTimeout = pings::MaxResolveTimeout + 1ms;
        expectValidity("resolve timeout: above maximum", options, false);

        options.resolveTimeout = -1ms;
        expectValidity("resolve timeout: negative", options, false);
    }

    // payloadSize
    {
        pings::PingOptions options{};

        options.payloadSize = 0;
        expectValidity("payload: zero", options, true);

        options.payloadSize = pings::MaxPayloadSize;
        expectValidity("payload: maximum", options, true);

        options.payloadSize = pings::MaxPayloadSize + 1;
        expectValidity("payload: above maximum", options, false);
    }

    {
        pings::PingOptions options{};

        options.addressFamily = pings::AddressFamily::Any;
        expectValidity("family: any", options, true);

        options.addressFamily = pings::AddressFamily::IPv4;
        expectValidity("family: IPv4", options, true);

        options.addressFamily = pings::AddressFamily::IPv6;
        expectValidity("family: IPv6", options, true);

        options.addressFamily =
            static_cast<pings::AddressFamily>(999);

        expectValidity("family: invalid value", options, false);
    }

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}