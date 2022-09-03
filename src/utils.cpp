#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>

#include "utils.h"

utils::Ncurses::Ncurses() {
    initscr();
    raw();
    noecho();
    curs_set(0);
}

utils::Ncurses::~Ncurses() {
    endwin();
}

utils::Ncurses::Screen::Screen(int height, int width, int y, int x) : lines_(height - 1), cols_(width - 1) {
    window_ = newwin(height, width, y, x);
    if (window_ == nullptr) throw std::runtime_error("Failed to create a window");
    keypad(window_, true);
}

utils::Ncurses::Screen::~Screen() {
    delwin(window_);
}

auto utils::Ncurses::Screen::getCharLower() -> char {
    return static_cast<char>(std::tolower(wgetch(window_)));
}

void utils::Ncurses::Screen::putLineAt(const std::string &string, int y, int x) {
    wmove(window_, y, 0);
    wclrtoeol(window_);
    wmove(window_, y, x);
    waddstr(window_, string.c_str());
    wrefresh(window_);
}

void utils::Ncurses::Screen::putLineFor(const std::string &string, int y, int x, std::chrono::seconds duration) {
    putLineAt(string, y, x);
    wrefresh(window_);
    std::this_thread::sleep_for(duration);
    wmove(window_, y, 0);
    wclrtoeol(window_);
    wrefresh(window_);
}

void utils::Ncurses::Screen::putLineWrapped(const std::string &string, int y, int x, int width) {
    auto strLen{string.length()};
    for (auto i = 0, l = 0; i < strLen; l++) {
        auto charWrappedLine = string.substr(i, width);
        auto lastCharIdx{charWrappedLine.length() - 1};
        if (charWrappedLine[lastCharIdx] != '\n' && charWrappedLine[lastCharIdx] != ' ') {
            auto lastSpace{charWrappedLine.find_last_of(' ')};
            auto wordWrappedLine = charWrappedLine.substr(0, lastSpace);
            i += static_cast<int>(wordWrappedLine.size());
            putLineAt(wordWrappedLine, y + l, x);
        } else {
            putLineAt(charWrappedLine, y + l, x);
            i += width;
        }
    }
}


auto
utils::Ncurses::Screen::ask(const std::string &string, const std::string &validChars, unsigned int retries) -> char {
    char ans;
    while (retries--) {
        putLineAt(string, 0, 0);
        ans = getCharLower();
        if (validChars.find(ans) == std::string::npos) {
            putLineFor(std::string(" is not a valid answer").insert(0, 1, ans), lines_ / 2, cols_ / 2 - 11,
                       std::chrono::seconds(1));
            ans = '\0';
        }
    }
    return ans;
}

void utils::Ncurses::Screen::clearScreen() {
    wclear(window_);
}

auto utils::Ncurses::Screen::getLines() const -> int {
    return lines_;
}

auto utils::Ncurses::Screen::getCols() const -> int {
    return cols_;
}

utils::Ncurses::Screen::Screen(WINDOW *window) : lines_(LINES - 1), cols_(COLS - 1) {
    window_ = window;
    keypad(window_, true);
}

std::string utils::executeProcess(const std::string &path, const std::vector<const char *> &args) noexcept(false) {
    int fields[2];  // 0: read fd, 1: write fd
    pipe(fields);
    char buf[256]{0};
    std::string output;
    auto pid{vfork()};
    switch (pid) {
        case -1:
            throw std::runtime_error("Failed to fork");
        case 0:
            dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
            dup2(fields[1], STDOUT_FILENO);
            dup2(fields[1], STDERR_FILENO);
            execv(path.c_str(), (char *const *) (args.begin().base()));     // shouldn't return
            break;
        default:
            // TODO: handle errors
            read(fields[0], buf, sizeof(buf));
            waitpid(pid, nullptr, 0);
            output.append(buf);
            if (output.empty()) throw std::runtime_error("Failed to run " + path);
            break;
    }
    output.append(1, '\n');                             // make sure we have a line end
    return output.substr(0, output.find('\n') + 1);     // get one line from output
}


std::unique_ptr<char>
utils::WavReader::loadWAV(const std::string &audioFile, unsigned int &chan, unsigned int &sampleRate, unsigned int &bps,
                          unsigned int &size) {
    auto file = std::unique_ptr<FILE, decltype(&fclose)>(std::fopen(audioFile.c_str(), "r"), fclose);
    struct WavHeader wavHeader{};
    std::fread(&wavHeader, 1, sizeof(wavHeader), file.get());
    chan = wavHeader.numOfChan;
    sampleRate = wavHeader.samplesPerSec;
    bps = wavHeader.bitsPerSample;
    size = wavHeader.subChunk2Size;
    auto buffer = std::unique_ptr<char>(new char[size]);
    std::fread(buffer.get(), sizeof(char), size, file.get());
    return buffer;
}