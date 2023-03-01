#pragma once

#include <chrono>
#include "utils.h"

enum TimewCommand {
    NONE, START, STOP, RESUME, QUERY
};


template<typename Rep, typename Period>
struct PomodoroSession {
    std::chrono::duration<Rep, Period> focusDuration;
    std::chrono::duration<Rep, Period> breakDuration;
    TimewCommand timewCommand;
};

struct TimewQueryResult {
    std::chrono::seconds trackedTime;
    std::string taskDescription;
    bool isTracking;
};

class Timew {
public:
    static utils::ProcessResult start(std::vector<std::string> &tags) noexcept(false) {
        throw std::logic_error("start not implemented");
    }

    static utils::ProcessResult stop() noexcept(false) {
        return utils::executeProcess("/usr/bin/timew", {"stop", ":adjust", nullptr});
    }

    static utils::ProcessResult resume() noexcept(false) {
        return utils::executeProcess("/usr/bin/timew", {"continue", nullptr});
    }

    static TimewQueryResult query() noexcept(false) {
        auto result{utils::executeProcess("/usr/bin/timew", {nullptr})};

        if (result.exitCode != 0) {
            return {std::chrono::seconds(0), ""};
        }

        auto targetIdx{result.output.find("Total")};
        if (targetIdx == std::string::npos) {
            throw std::runtime_error("Failed to parse timew command output");
        }

        targetIdx += 5;     // skip "Total"
        while (targetIdx < result.output.length() && std::isspace(result.output[targetIdx]))
            ++targetIdx;

        char *separatorIdx{nullptr};
        auto hours{std::strtoul(&result.output[targetIdx], &separatorIdx, 10)};
        auto minutes{std::strtoul(separatorIdx + 1, &separatorIdx, 10)};
        auto seconds{std::strtoul(separatorIdx + 1, nullptr, 10)};

        return {std::chrono::seconds(hours * 60 * 60 + minutes * 60 + seconds),
                utils::formatDescription(result.output), result.exitCode == 0};
    }
};
