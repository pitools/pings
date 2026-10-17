#include <pings/repeat_handle.hpp>
#include <pings/async_pinger.hpp>

namespace pings
{
    RepeatHandle::RepeatHandle(
        std::weak_ptr<AsyncPinger> owner,
        std::uint64_t id)
        : owner_(std::move(owner)),
          id_(id)
    {
    }

    void RepeatHandle::cancel()
    {
        if (auto owner = owner_.lock())
        {
            owner->requestCancelRepeat(id_);
        }
    }

    bool RepeatHandle::valid() const noexcept
    {
        return !owner_.expired();
    }
}