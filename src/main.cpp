#include <future>
#include <iostream>

#include "utils.h"
#include "config.h"
#include "Ncurses.h"
#include "sound/AudioPlayer.h"

auto main() -> int {
    Ncurses ncurses;     // handle initialization of ncurses
    Ncurses::Screen cmdScreen(stdscr);
    Ncurses::Screen tmrScreen(LINES - 2, COLS - 2, 1, 1);
    AudioPlayer audioPlayer;
    utils::Queue<PomodoroSession<long, std::nano>> taskQueue;
    std::atomic<bool> isRunning = true;
    std::atomic<bool> isPause = false;

    std::thread worker([&] {
        audioPlayer.load(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Synth_Brass.ogg");
        audioPlayer.load(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Retro_Synth.ogg");
        std::chrono::duration<int, std::nano> delta(0);
        while (isRunning) {
            auto task{taskQueue.pop()};
            auto prevTime{std::chrono::steady_clock::now()};
            std::wstring taskDescription;
            if (task.sessionType == PomodoroSessionType::WORK) {
                try {
                    taskDescription = utils::executeProcess("/usr/bin/timew", {"continue", nullptr});
                } catch (const std::runtime_error &error) {
                    cmdScreen.putLineFor(utils::toWstring(error.what()), cmdScreen.getLines(), 0,
                                         std::chrono::seconds(1));
                    continue;
                }
            }
            // timer
            while (task.duration.count() > 0 && isRunning && !isPause) {
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
            if (task.sessionType == PomodoroSessionType::WORK) {
                try {
                    utils::executeProcess("/usr/bin/timew", {"stop", nullptr});
                } catch (const std::runtime_error &error) {
                    cmdScreen.putLineFor(utils::toWstring(error.what()), cmdScreen.getLines(), 0, std::chrono::seconds(1));
                }
                audioPlayer.play(PROJECT_INSTALL_PREFIX "/share/" PROJECT_NAME "/sounds/Retro_Synth.ogg");
            } else if (task.sessionType == PomodoroSessionType::FREE) {
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
                    isPause = false;
                    taskQueue.push({std::chrono::duration<long, std::ratio<60, 1>>(25), PomodoroSessionType::WORK});
                    taskQueue.push({std::chrono::duration<long, std::ratio<60, 1>>(5), PomodoroSessionType::FREE});
                } else {
                    cmdScreen.putLineFor(L"Timer is already running", cmdScreen.getLines(), 0, std::chrono::seconds(1));
                }
                break;
            case 'p':
                isPause = true;
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

    isRunning = false;
    taskQueue.push({});    // necessary since the thread waits on the queue
    worker.join();

    return 0;
}