#include <future>
#include <iostream>
#include <csignal>

#include "utils.h"
#include "config.h"
#include "Ncurses.h"
#include "sound/AudioPlayer.h"

static utils::Queue<PomodoroSession<long, std::nano>> taskQueue;
static std::atomic<bool> isRunning = true;
static std::atomic<bool> isPause = false;

auto usr1SigHandler(int) {
    taskQueue.push({std::chrono::duration<long, std::ratio<60, 1>>(25), SessionType::WORK, CommandType::QUERY});
    taskQueue.push({std::chrono::duration<long, std::ratio<60, 1>>(5), SessionType::FREE});
    isPause.store(false, std::memory_order_relaxed);    // not used for synchronization
}

auto main() -> int {
    Ncurses ncurses;     // handle initialization of ncurses
    Ncurses::Screen cmdScreen(stdscr);
    Ncurses::Screen tmrScreen(LINES - 2, COLS - 2, 1, 1);
    AudioPlayer audioPlayer;

    struct sigaction sa{.sa_flags = SA_RESTART | SA_NOCLDSTOP};
    sa.sa_handler = usr1SigHandler;
    if (sigaction(SIGUSR1, &sa, nullptr) == EINVAL) {
        cmdScreen.putLineFor(L"Unable to handle signals", cmdScreen.getLines() - 1, cmdScreen.getCols() / 2 - 12,
                             std::chrono::seconds(1));
    }

    std::thread worker([&] {
        audioPlayer.load(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Synth_Brass.ogg");
        audioPlayer.load(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Retro_Synth.ogg");

        std::chrono::duration<int, std::nano> delta(0);
        while (isRunning.load(std::memory_order_relaxed)) {
            auto task{taskQueue.pop()};
            auto prevTime{std::chrono::steady_clock::now()};
            std::wstring taskDescription;

            if (task.sessionType == SessionType::WORK) {
                try {
                    if (task.commandType == CommandType::CONTINUE)
                        taskDescription = utils::executeProcess("/usr/bin/timew", {"continue", nullptr});
                    else if (task.commandType == CommandType::QUERY)
                        taskDescription = utils::executeProcess("/usr/bin/timew", {nullptr});
                } catch (const std::runtime_error &error) {
                    cmdScreen.putLineFor(utils::toWstring(error.what()), cmdScreen.getLines(), 0,
                                         std::chrono::seconds(1));
                    continue;
                }
            }
            // timer
            while (isRunning.load(std::memory_order_relaxed) && !isPause.load(std::memory_order_relaxed) &&
                   task.duration.count() > 0) {
                // TODO: use ascii art to print digits adapted to the size of the terminal
                cmdScreen.putLineAt(L"commands: (s)tart, (p)ause, (e)xit", 0, cmdScreen.getCols() / 2 - 17);
                tmrScreen.putLineAt(utils::formatSeconds<long, std::nano>(task.duration), 1,
                                    tmrScreen.getCols() / 2 - 5);
                tmrScreen.putLineWrapped(taskDescription, tmrScreen.getLines() - 3,
                                         static_cast<int>(tmrScreen.getCols() / 2 - taskDescription.length() / 2),
                                         tmrScreen.getCols() - 2);
                auto sleepTime{std::chrono::seconds(1) - delta};            // desired sleep time
                std::this_thread::sleep_for(std::chrono::duration<long, std::nano>(sleepTime));
                auto curTime = std::chrono::steady_clock::now();
                auto timeSlept = curTime - prevTime;                        // actual slept time
                delta = (timeSlept - sleepTime) % std::chrono::seconds(1);  // fraction of second
                task.duration -= timeSlept;
                prevTime = curTime;
            }

            if (task.sessionType == SessionType::WORK) {
                try {
                    utils::executeProcess("/usr/bin/timew", {"stop", nullptr});
                } catch (const std::runtime_error &error) {
                    cmdScreen.putLineFor(utils::toWstring(error.what()), cmdScreen.getLines(), 0,
                                         std::chrono::seconds(1));
                }
                audioPlayer.play(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Retro_Synth.ogg");
            } else if (task.sessionType == SessionType::FREE) {
                audioPlayer.play(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Synth_Brass.ogg");
            }
            tmrScreen.clear();
        }
    });


    int cmdChar;
    cmdScreen.putLineAt(L"commands: (s)tart, (p)ause, (e)xit", 0, cmdScreen.getCols() / 2 - 17);
    while ((cmdChar = cmdScreen.getCharToLower()) != 'e') {
        switch (cmdChar) {
            case 's':
                if (taskQueue.empty()) {
                    taskQueue.push({std::chrono::duration<long, std::ratio<60, 1>>(25), SessionType::WORK,
                                    CommandType::CONTINUE});
                    taskQueue.push({std::chrono::duration<long, std::ratio<60, 1>>(5), SessionType::FREE});
                    isPause.store(false, std::memory_order_relaxed);    // not used for synchronization
                } else {
                    cmdScreen.putLineFor(L"Timer is already running", cmdScreen.getLines(), 0, std::chrono::seconds(1));
                }
                break;
            case 'p':
                isPause.store(true, std::memory_order_relaxed);
                break;
            case KEY_RESIZE:
                int lines, cols;
                getmaxyx(stdscr, lines, cols);
                cmdScreen.resize(lines, cols);
                tmrScreen.resize(lines - 2, cols - 2);
                cmdScreen.putLineAt(L"commands: (s)tart, (p)ause, (e)xit", 0, cmdScreen.getCols() / 2 - 17);
                break;
            default:
                break;
        }
        flushinp();
    }

    taskQueue.push({});    // necessary since the thread waits on the queue
    isRunning.store(false, std::memory_order_relaxed);  // not used for synchronization
    worker.join();

    return 0;
}