#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <repeat_statistics.hpp>

namespace
{
    int checks = 0;
    int failures = 0;

    void expect(bool condition, std::string_view name)
    {
        ++checks;

        if (!condition)
        {
            ++failures;
            std::cerr << "FAIL: " << name << '\n';
        }
    }
}

int main()
{
    using namespace std::chrono_literals;
    using pings::PingResult;
    using pings::PingStatistics;
    using pings::PingStatus;
    using pings::detail::recordRepeatResult;

    PingStatistics statistics{};
    std::chrono::milliseconds totalLatency{};

    // DNS-ошибка: попытка есть, отправленного пакета нет.
    PingResult dnsFailure{};
    dnsFailure.status = PingStatus::ResolveError;

    recordRepeatResult(statistics, totalLatency, dnsFailure);

    expect(
        statistics.attempts == 1 &&
            statistics.sent == 0 &&
            statistics.received == 0 &&
            statistics.lost == 0,
        "DNS failure is an attempt, not packet loss");

    expect(
        statistics.lossPercent == 0.0,
        "no sent packets means zero loss percentage");

    expect(
        totalLatency == 0ms &&
            statistics.min == 0ms &&
            statistics.max == 0ms &&
            statistics.avg == 0ms,
        "DNS failure does not produce RTT measurements");

    // Первый ответ.
    PingResult success{};
    success.status = PingStatus::Success;
    success.requestSent = true;
    success.latency = 40ms;

    recordRepeatResult(statistics, totalLatency, success);

    expect(
        statistics.attempts == 2 &&
            statistics.sent == 1 &&
            statistics.received == 1 &&
            statistics.lost == 0,
        "first success counters");

    expect(
        statistics.min == 40ms &&
            statistics.max == 40ms &&
            statistics.avg == 40ms,
        "first success initializes RTT");

    // Второй ответ быстрее первого.
    success.latency = 20ms;

    recordRepeatResult(statistics, totalLatency, success);

    expect(
        statistics.min == 20ms &&
            statistics.max == 40ms &&
            statistics.avg == 30ms &&
            totalLatency == 60ms,
        "RTT aggregate for two replies");

    // Пакет отправлен, но ответ не получен.
    PingResult timeout{};
    timeout.status = PingStatus::Timeout;
    timeout.requestSent = true;

    recordRepeatResult(statistics, totalLatency, timeout);

    expect(
        statistics.attempts == 4 &&
            statistics.sent == 3 &&
            statistics.received == 2 &&
            statistics.lost == 1,
        "timeout counts as packet loss");

    expect(
        std::abs(statistics.lossPercent - 100.0 / 3.0) < 1e-9,
        "one lost packet out of three sent");

    expect(
        statistics.min == 20ms &&
            statistics.max == 40ms &&
            statistics.avg == 30ms &&
            totalLatency == 60ms,
        "timeout does not change RTT");

    // Ошибка отправки без подтверждённой отправки пакета.
    PingResult sendFailure{};
    sendFailure.status = PingStatus::SendError;

    recordRepeatResult(statistics, totalLatency, sendFailure);

    expect(
        statistics.attempts == 5 &&
            statistics.sent == 3 &&
            statistics.received == 2 &&
            statistics.lost == 1,
        "unsent request does not increase packet loss");

    // Нулевой RTT — допустимое измерение после округления.
    PingStatistics zeroStatistics{};
    std::chrono::milliseconds zeroTotal{};

    success.latency = 0ms;
    recordRepeatResult(zeroStatistics, zeroTotal, success);

    expect(
        zeroStatistics.received == 1 &&
            zeroStatistics.min == 0ms &&
            zeroStatistics.max == 0ms &&
            zeroStatistics.avg == 0ms,
        "zero RTT is a valid measurement");

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}