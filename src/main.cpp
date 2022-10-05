#include <future>
#include <iostream>
#include <csignal>

#include "utils.h"
#include "config.h"
#include "Ncurses.h"
#include "sound/AudioPlayer.h"

constexpr int tmrScreenLines = 2;

static utils::concurrent::queue<utils::PomodoroSession<int64_t, std::nano>> taskQueue;
static std::atomic<bool> isRunning = true;
static std::atomic<bool> isPause = true;

static auto usr1SigHandler(int) {
    if (isPause.load(std::memory_order::relaxed))
        taskQueue.push({std::chrono::minutes(25), std::chrono::minutes(5), utils::TimewCommand::QUERY});
}

template<typename Rep, typename Period>
static auto countDown(const Ncurses::Screen &tmrScreen, const Ncurses::Screen &cmdScreen, const std::string &title,
                      const std::string &taskDescription, std::chrono::duration<Rep, Period> duration) {
    std::chrono::duration<int64_t, std::nano> delta(0);
    auto prevTime{std::chrono::steady_clock::now()};

    while (isRunning.load(std::memory_order_relaxed) && !isPause.load(std::memory_order_relaxed) &&
           duration.count() > 0) {
        std::string secRep{utils::formatSeconds<Rep, Period>(duration)};
        tmrScreen.putCentered(title, 0, static_cast<int>(title.size()));
        tmrScreen.putCentered(secRep, 1, static_cast<int>(secRep.size()));
        cmdScreen.putCentered(taskDescription, cmdScreen.getLines() - 2, cmdScreen.getCols() - 11);
        auto sleepTime{std::chrono::seconds(1) - delta};
        std::this_thread::sleep_for(sleepTime);
        auto curTime{std::chrono::steady_clock::now()};
        auto timeSlept{curTime - prevTime};
        delta = (timeSlept - sleepTime) % std::chrono::seconds(1);
        duration -= timeSlept;
        prevTime = curTime;
    }

    return duration.count() <= 0;
}

auto main() -> int {
    AudioPlayer audioPlayer;            // handle initialization of audio player
    Ncurses ncurses;                    // handle initialization of ncurses
    Ncurses::Screen cmdScreen(stdscr);
    Ncurses::Screen tmrScreen(tmrScreenLines, COLS, 2, 0);

    std::thread worker([&] {
        audioPlayer.load(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Synth_Brass.ogg");
        audioPlayer.load(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Retro_Synth.ogg");

        while (isRunning.load(std::memory_order_relaxed)) {
            auto task{taskQueue.wait_pop()};
            if (task.timewCommand == utils::TimewCommand::NONE) break;
            isPause.store(false, std::memory_order_relaxed);

            std::string taskDesc;
            try {
                if (task.timewCommand == utils::TimewCommand::CONTINUE)
                    taskDesc = utils::formatDescription(utils::executeProcess("/usr/bin/timew", {"continue", nullptr}));
                else if (task.timewCommand == utils::TimewCommand::QUERY)
                    taskDesc = utils::formatDescription(utils::executeProcess("/usr/bin/timew", {nullptr}));
            } catch (const std::runtime_error &error) {
                cmdScreen.putFor(error.what(), cmdScreen.getLines() - 1, 0, std::chrono::seconds(1));
                continue;
            }

            if (!countDown(tmrScreen, cmdScreen, "Focus!", taskDesc, task.focusDuration)) continue;

            try {
                audioPlayer.play(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Retro_Synth.ogg");
                utils::executeProcess("/usr/bin/timew", {"stop", nullptr});
            } catch (const std::runtime_error &error) {
                cmdScreen.putFor(error.what(), cmdScreen.getLines() - 1, 0, std::chrono::seconds(1));
            }

            if (!countDown(tmrScreen, cmdScreen, "Break!", taskDesc, task.breakDuration)) continue;

            isPause.store(true, std::memory_order_relaxed);
            audioPlayer.play(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Synth_Brass.ogg");
        }
    });

    struct sigaction sa{.sa_flags = SA_RESTART | SA_NOCLDSTOP};
    sa.sa_handler = usr1SigHandler;
    if (sigaction(SIGUSR1, &sa, nullptr) == EINVAL) {
        PUT_CENTERED_FOR(cmdScreen, "Unable to handle signals", cmdScreen.getLines() - 1, std::chrono::seconds(1));
    }

    int cmdChar;
    PUT_CENTERED(cmdScreen, "commands: (s)tart, (p)ause, (e)xit", 0);
    while ((cmdChar = cmdScreen.getCharToLower()) != 'e') {
        switch (cmdChar) {
            case 's':
                if (isPause.load(std::memory_order::relaxed)) {
                    taskQueue.push({std::chrono::minutes(25), std::chrono::minutes(5), utils::TimewCommand::CONTINUE});
                } else {
                    cmdScreen.putFor("Timer is already running", cmdScreen.getLines() - 1, 0, std::chrono::seconds(1));
                }
                break;
            case 'p':
                isPause.store(true, std::memory_order_relaxed);
                break;
            case KEY_RESIZE:
                int lines, cols;
                getmaxyx(stdscr, lines, cols);
                cmdScreen.resize(lines, cols);
                tmrScreen.resize(tmrScreenLines, cols);
                PUT_CENTERED(cmdScreen, "commands: (s)tart, (p)ause, (e)xit", 0);
                break;
            default:
                break;
        }
        flushinp();
    }

    isRunning.store(false, std::memory_order_relaxed);  // not used for synchronization
    taskQueue.push({});    // necessary since the thread waits on the queue
    worker.join();

    return 0;
}