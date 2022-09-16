#pragma once

#include <queue>
#include <condition_variable>

#ifndef __ANDROID__

#include <AL/al.h>

#endif

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
            pendingData_.wait(lk, [this] { return queue_.size() > 0; });
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
    std::wstring formatSeconds(const std::chrono::duration<Rep, Period> &duration) {
        auto seconds{std::chrono::duration_cast<std::chrono::seconds>(duration).count()};
        if (seconds < 0) throw std::invalid_argument("seconds can't be negative");
        std::tm time{};
        time.tm_sec = seconds % 60;
        seconds /= 60;
        time.tm_min = seconds % 60;
        seconds /= 60;
        time.tm_hour = seconds % 60;
        wchar_t buf[64];
        std::wcsftime(buf, sizeof(buf), L"%H:%M:%S", &time);
        return {buf};
    }

    std::wstring executeProcess(const std::string &path, const std::vector<const char *> &args) noexcept(false);

    std::wstring toWstring(const std::string &string);

    std::string toString(const std::wstring &wstring);

    class WavReader {
    public:
        static std::unique_ptr<char>
        loadWAV(const std::string &audioFile, unsigned int &chan, unsigned int &sampleRate, unsigned int &bps,
                unsigned int &size);

    private:
        struct WavHeader {
            /* RIFF Chunk Descriptor */
            [[maybe_unused]] uint8_t riff[4];
            [[maybe_unused]] uint32_t chunkSize;
            [[maybe_unused]] uint8_t waveHeader[4];
            /* "fmt" sub-chunk */
            [[maybe_unused]] uint8_t fmt[4];
            [[maybe_unused]] uint32_t subChunk1Size;
            [[maybe_unused]] uint16_t audioFormat;
            [[maybe_unused]] uint16_t numOfChan;
            [[maybe_unused]] uint32_t samplesPerSec;
            [[maybe_unused]] uint32_t bytesPerSec;
            [[maybe_unused]] uint16_t blockAlign;
            [[maybe_unused]] uint16_t bitsPerSample;
            /* "data" sub-chunk */
            [[maybe_unused]] uint8_t subChunk2ID[4];
            [[maybe_unused]] uint32_t subChunk2Size;
        };
    };

#ifndef __ANDROID__

    inline unsigned int getAlAudioFormat(unsigned int channel, unsigned int bps) {
        if (channel == 1) {
            if (bps == 8) {
                return AL_FORMAT_MONO8;
            } else {
                return AL_FORMAT_MONO16;
            }
        } else {
            if (bps == 8) {
                return AL_FORMAT_STEREO8;
            } else {
                return AL_FORMAT_STEREO16;
            }
        }
    }

#endif
}