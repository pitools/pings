#pragma once

#include <cstdint>
#include <memory>

namespace pings
{
    namespace detail
    {
        struct PingSession;
    }

    class AsyncPinger;

    class PingHandle
    {
    public:
        PingHandle() = default;
        void cancel();

        // Reports whether the owning pinger still exists.
        // Does not indicate whether the ping operation is still active.
        [[nodiscard]]
        bool valid() const noexcept;

    private:
        friend class AsyncPinger;

        PingHandle(std::weak_ptr<AsyncPinger> owner, std::weak_ptr<detail::PingSession> session);

        std::weak_ptr<AsyncPinger> owner_;
        std::weak_ptr<detail::PingSession> session_;
    };
}