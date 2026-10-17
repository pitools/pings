#include <pings/batch_handle.hpp>

namespace pings
{
    BatchHandle::BatchHandle(std::shared_ptr<std::vector<PingHandle>> handles)
        : handles_(std::move(handles))
    {
    }

    void BatchHandle::cancel()
    {
        if (!handles_)
        {
            return;
        }

        for (auto &handle : *handles_)
        {
            handle.cancel();
        }
    }

    bool BatchHandle::valid() const noexcept
    {
        return static_cast<bool>(handles_);
    }
}