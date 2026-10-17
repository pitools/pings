#include <utility>

#include <pings/ping_handle.hpp>
#include <pings/async_pinger.hpp>

namespace pings
{
        void PingHandle::cancel()
        {
            if (auto owner = owner_.lock())
            {
                owner->requestCancel(session_);
            }
        }

        // AsyncPinger ещё существует
        [[nodiscard]]
        bool PingHandle::valid() const noexcept
        {
            return !owner_.expired();
        }

        PingHandle::PingHandle(std::weak_ptr<AsyncPinger> owner, std::weak_ptr<detail::PingSession> session)
            : owner_(std::move(owner)),
              session_(std::move(session))
        {
        }
}