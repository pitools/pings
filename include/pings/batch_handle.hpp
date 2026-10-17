#pragma once

#include <memory>
#include <vector>

#include <pings/ping_handle.hpp>

namespace pings
{
    class BatchHandle
    {
    public:
        BatchHandle() = default;

        void cancel();

        [[nodiscard]]
        bool valid() const noexcept;

    private:
        friend class AsyncPinger;

        explicit BatchHandle(std::shared_ptr<std::vector<PingHandle>> handles);

        std::shared_ptr<std::vector<PingHandle>> handles_;
    };
}