#pragma once

#include <ncurses.h>
#include <string>
#include <queue>
#include <condition_variable>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <fcntl.h>

// TODO: make sessions' times configurable
constexpr unsigned int SESSION_TIME_SECS = 25 * 60;
constexpr unsigned int BREAK_TIME_SECS = 5 * 60;

enum PomodoroSessionType {
    FREE = 1, WORK
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
        explicit Ncurses();

        ~Ncurses();

        class Screen {
        public:
            explicit Screen(WINDOW *window) noexcept(false);

            Screen(int height, int width, int y, int x) noexcept(false);

            ~Screen();

            char getCharLower();

            int getLines() const;

            int getCols() const;

            void putLineAt(const std::string &string, int y, int x);

            void putLineFor(const std::string &string, int y, int x, std::chrono::seconds duration);

            void putLineWrapped(const std::string &string, int y, int x, int width);

            char ask(const std::string &string, const std::string &validChars, unsigned int retries);

            void clearScreen();

        private:
            WINDOW *window_;
            int lines_;         // number of lines -1 (to be used as index)
            int cols_;          // number of columns - 1 (to be use ad index)
        };
    };

    class SDL2 {
    public:
        SDL2() noexcept(false);

        ~SDL2();

        class Sample {
        public:
            explicit Sample(const std::string &musicFile) noexcept(false);

            void play();

        private:
            std::unique_ptr<Mix_Music, void (*)(Mix_Music *)> musicPtr_;
        };

    private:
        bool isInitialized_ = false;
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
    }

    std::string executeProcess(const std::string &path, const std::vector<const char *> &args) noexcept(false);
}