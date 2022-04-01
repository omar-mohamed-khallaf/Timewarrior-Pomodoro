#pragma once

#include <ncurses.h>
#include <string>
#include <queue>
#include <condition_variable>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// TODO: make sessions' times configurable
constexpr unsigned int SESSION_TIME_SECS = 25 * 60;
constexpr unsigned int BREAK_TIME_SECS = 5 * 60;

enum PomodoroSessionType {
    FREE, WORK
};

template<typename Rep, typename Period>
struct PomodoroSession {
public:
    std::chrono::duration<Rep, Period> duration;
    PomodoroSessionType sessionType;
};

namespace utils {
    class Ncurses {
    public:
        explicit Ncurses() {
            initscr();
        }

        ~Ncurses() {
            endwin();
        }

        class Screen {
        public:
            explicit Screen(WINDOW *window) : lines_(LINES), cols_(COLS) {
                raw();
                noecho();
                keypad(stdscr, true);
                curs_set(0);
                window_ = window;
            }

            Screen(int height, int width, int y, int x) : lines_(height), cols_(width) {
                raw();
                noecho();
                keypad(stdscr, true);
                curs_set(0);
                window_ = newwin(height, width, y, x);
            }

            ~Screen() {
                delwin(window_);
            }

            char getCharLower();

            int getLines() const;

            int getCols() const;

            void putStringAtLine(const std::string &string, int y, int x);

            template<typename Rep, typename Period>
            void putStringAtFor(const std::string &string, int y, int x, std::chrono::duration<Rep, Period> duration) {
                putStringAtLine(string, y, x);
                wrefresh(window_);
                std::this_thread::sleep_for(duration);
                move(y, 0);
                wclrtoeol(window_);
            };

            char ask(const std::string &string, const std::string &validChars, unsigned int retries);

            void clearScreen();

            void render();

        private:
            WINDOW *window_;
            int lines_;
            int cols_;
        };
    };

    class SDL2 {
    public:
        SDL2() noexcept(false) {
            if (isInitialized) return;
            if (SDL_Init(SDL_INIT_AUDIO) < 0) throw std::runtime_error("Failed to initialize SDL2");
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
                throw std::runtime_error("Failed to initialize SDL2 mixer");
            isInitialized = true;
        }

        ~SDL2() {
            Mix_Quit();
            SDL_Quit();
            isInitialized = false;
        }

        class Sample {
        public:
            explicit Sample(const std::string &musicFile) : music(Mix_LoadMUS(musicFile.c_str()), Mix_FreeMusic) {
                if (music == nullptr)
                    throw std::runtime_error(std::string("Failed to play music: ").append(Mix_GetError()));
            }

            void play() {
                Mix_PlayMusic(music.get(), 0);
            }

        private:
            std::unique_ptr<Mix_Music, void (*)(Mix_Music *)> music;
        };

    private:
        bool isInitialized = false;
    };

    template<typename T>
    class Queue {
    public:
        Queue() = default;

        Queue(const Queue &) = delete;

        Queue(Queue &&) = delete;

        void push(T x) {
            std::unique_lock lk(mutex_);
            queue_.push(x);
            // Manual unlocking is done before notifying, to avoid waking up the waiting thread only to block again
            lk.unlock();
            pendingData_.notify_one();
        };

        T pop() {
            std::unique_lock lk(mutex_);
            pendingData_.wait(lk, [this] { return this->queue_.size() > 0; });
            T ref = queue_.front();
            queue_.pop();
            return ref;
        };

        bool empty() {
            std::lock_guard lk(mutex_);
            return queue_.empty();
        };

    private:
        std::queue<T> queue_;
        std::condition_variable pendingData_;
        std::mutex mutex_;
    };

    template<typename Rep, typename Period>
    std::string formatSeconds(const std::chrono::duration<Rep, Period> &duration) {
        auto seconds{std::chrono::duration_cast<std::chrono::seconds>(duration).count()};
        if (seconds < 0) throw std::invalid_argument("seconds can't be negative");
        std::tm time{};
        time.tm_sec = seconds % 60;
        seconds /= 60;
        time.tm_min = seconds % 60;
        seconds /= 60;
        time.tm_hour = seconds % 60;
        char buf[64];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", &time);
        return {buf};
    };
}
