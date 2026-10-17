#include <utility>

#include <batch_state.hpp>

namespace pings::detail
{
    BatchState::BatchState(
        std::size_t size,
        BatchCompletionCallback completionCallback,
        BatchProgressCallback progressCallback)
        : results_(size),
          completed_(size, false),
          remaining_(size),
          total_(size),
          completionCallback_(std::move(completionCallback)),
          progressCallback_(std::move(progressCallback))
    {
    }

    void BatchState::complete(
        std::size_t index,
        const PingResult &result)
    {
        if (index >= results_.size())
        {
            return;
        }

        if (completed_[index])
        {
            return;
        }

        completed_[index] = true;
        results_[index] = result;

        --remaining_;

        const auto completedCount = total_ - remaining_;

        if (progressCallback_)
        {
            try
            {
                progressCallback_(completedCount, total_);
            }
            catch (...)
            {
                // Пользовательский callback не должен ломать batch state
            }
        }

        if (remaining_ != 0)
        {
            return;
        }

        auto completionCallback = std::move(completionCallback_);

        if (completionCallback)
        {
            try
            {
                completionCallback(results_);
            }
            catch (...)
            {
                // Пользовательский callback не должен разрушать event loop
            }
        }
    }
}