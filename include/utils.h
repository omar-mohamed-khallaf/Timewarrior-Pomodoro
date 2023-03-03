#pragma once

#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>

#ifndef __ANDROID__

#include <AL/al.h>
#include <cassert>

#endif


namespace utils {
    namespace concurrent {
        /**
         * A threadsafe queue inspired from the STL
         * @tparam Tp Type of element.
         * @tparam Sequence Type of underlying sequence, defaults to deque_Tp>
         * @tparam Mutex Type of mutex used to synchronize operations
         */
        template<typename Tp, typename Sequence = std::deque<Tp>, typename Mutex = std::mutex>
        class queue {
            template<typename Tp1, typename Seq1>
            friend bool operator==(const queue<Tp1, Seq1> &, const queue<Tp1, Seq1> &);

            template<typename Tp1, typename Seq1>
            friend bool operator<(const queue<Tp1, Seq1> &, const queue<Tp1, Seq1> &);

#ifdef __cpp_lib_three_way_comparison

            template<typename Tp1, std::three_way_comparable Seq1>
            friend std::compare_three_way_result_t<Seq1>
            operator<=>(const queue<Tp1, Seq1> &, const queue<Tp1, Seq1> &);

#endif
            template<typename Alloc> using Uses = typename std::enable_if<std::uses_allocator<Sequence, Alloc>::value>::type;


            static_assert(std::is_same<Tp, typename Sequence::value_type>::value,
                          "value_type must be the same as the underlying container");

        public:
            typedef typename Sequence::value_type value_type;
            typedef typename Sequence::reference reference;
            typedef typename Sequence::const_reference const_reference;
            typedef typename Sequence::size_type size_type;
            typedef Sequence container_type;

        private:
            Sequence c_;
            mutable Mutex m_;
            std::condition_variable cv_;

        public:
            template<typename Seq = Sequence, typename = typename std::enable_if<std::is_default_constructible<Seq>::value>::type>
            queue() : c_() {}

            explicit queue(const Sequence &_c) : c_(_c) {}

            explicit queue(Sequence &&_c) : c_(std::move(_c)) {}

            template<typename Alloc, typename = Uses<Alloc>>
            explicit queue(const Alloc &_a) : c_(_a) {}

            template<typename Alloc, typename = Uses<Alloc>>
            queue(Sequence &&_c, const Alloc &_a) : c_(std::move(_c), _a) {}

            template<typename Alloc, typename = Uses<Alloc>>
            queue(const queue &_q, const Alloc &_a) {
                std::scoped_lock(m_, _q.m_);
                c_(_q.c_, _a);
            }

            template<typename Alloc, typename = Uses<Alloc>>
            queue(queue &&_q, const Alloc &_a) {
                std::scoped_lock(m_, _q.m_);
                c_(std::move(_q.c_), _a);
            }

            bool empty() const {
                std::lock_guard lk(m_);
                return c_.empty();
            }

            size_type size() const {
                std::lock_guard lk(m_);
                return c_.size();
            }

            void push(const value_type &_x) {
                {
                    std::lock_guard lk(m_);
                    c_.push_back(_x);
                }
                cv_.notify_one();
            }

            void push(value_type &&_x) {
                {
                    std::lock_guard lk(m_);
                    c_.push_back(std::move(_x));
                }
                cv_.notify_one();
            }

            reference wait_pop() {
                std::unique_lock lk(m_);
                cv_.wait(lk, [&] { return !c_.empty(); });
                reference elem = c_.front();
                c_.pop_front();
                return elem;
            }

            void swap(queue &_q) noexcept(std::__is_nothrow_swappable<Sequence>::value) {
                using std::swap;
                std::scoped_lock(m_, _q.m_);
                swap(c_, _q.c_);
            }
        };

        template<typename Tp, typename Seq>
        inline bool operator==(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) {
            std::scoped_lock(_x.m_, _y.m_);
            return _x.c_ == _y.c_;
        }

        template<typename Tp, typename Seq>
        inline bool operator<(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) {
            std::scoped_lock(_x.m_, _y.m_);
            return _x.c_ < _y.c_;
        }

        template<typename Tp, typename Seq>
        inline bool operator!=(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) { return !(_x == _y); }

        template<typename Tp, typename Seq>
        inline bool operator>(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) { return _y < _x; }

        template<typename Tp, typename Seq>
        inline bool operator<=(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) { return !(_y < _x); }

        template<typename Tp, typename Seq>
        inline bool operator>=(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) { return !(_x < _y); }

#if __cpp_lib_three_way_comparison

        template<typename Tp, std::three_way_comparable Seq>
        inline std::compare_three_way_result_t<Seq> operator<=>(const queue<Tp, Seq> &_x, const queue<Tp, Seq> &_y) {
            std::scoped_lock(_x.m_, _y.m_);
            return _x.c_ <=> _y.c_;
        }

#endif

        template<typename Tp, typename Seq>
        inline typename std::enable_if<std::__is_swappable<Seq>::value>::type
        swap(queue<Tp, Seq> &_x, queue<Tp, Seq> &_y) noexcept(noexcept(_x.swap(_y))) { _x.swap(_y); }
    }

    /**
     * Reader for .wav files
     */
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

    struct ProcessResult {
        uint8_t exitCode;
        std::string output;
    };

    /**
     * Executes a process and returns stdout or stderr as string
     * @param path The path to the executable
     * @param args The arguments to path to the executable
     * @return ProcessResult struct containing the output and the exit code
     */
    ProcessResult executeProcess(const std::string &path, const std::vector<const char *> &args) noexcept(false);

    /**
     * Formats the stdout of timew commands
     * @param description The string returned from execute process
     * @return The new formatted string
     */
    std::string formatDescription(const std::string &description);

    /**
     * Converts std::string to std::wstring
     * @param string To be converted to std::wstring
     * @return std::wstring
     */
    std::wstring stringToUtf(const std::string &string);

    /**
     * Converts std::wstring to std::string
     * @param wstring To be converted to std::string
     * @return std::string
     */
    std::string utfToString(const std::wstring &wstring);

    /**
     * Formats a duration as seconds (e.g. 00:00:00)
     * @tparam Rep The type representing the period
     * @tparam Period The period of time represented
     * @param duration The duration to represent as seconds
     * @return a string in the format "%H:%M:%S"
     */
    template<typename Rep, typename Period>
    inline std::string formatSeconds(const std::chrono::duration<Rep, Period> &duration) {
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
}