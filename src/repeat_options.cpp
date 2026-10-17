#include <pings/repeat_options.hpp>

namespace pings
{
    bool isValid(const RepeatOptions &options) noexcept
    {
        if (!isValid(options.ping))
        {
            return false;
        }

        if (options.interval < MinRepeatInterval)
        {
            return false;
        }

        if (options.interval > MaxRepeatInterval)
        {
            return false;
        }

        return true;
    }
}