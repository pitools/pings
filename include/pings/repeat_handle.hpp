#pragma once

#include <cstdint>
#include <memory>

namespace pings
{
    class AsyncPinger;

    class RepeatHandle
    {
    public:
        RepeatHandle() = default;

        void cancel();

        // Проверяет, связан ли handle с существующим AsyncPinger
        // Не гарантирует, что ping-операция всё ещё активна
        [[nodiscard]]
        bool valid() const noexcept;

    private:
        friend class AsyncPinger;

        RepeatHandle(
            std::weak_ptr<AsyncPinger> owner,
            std::uint64_t id);

        std::weak_ptr<AsyncPinger> owner_;
        std::uint64_t id_{};
    };
}
