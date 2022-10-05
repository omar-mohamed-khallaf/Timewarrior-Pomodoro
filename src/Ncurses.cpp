#include <thread>
#include <locale>
#include <vector>

#include <algorithm>

#include "Ncurses.h"

#ifndef waddwstr
#include "utils.h"
#endif

Ncurses::Ncurses() {
    setlocale(LC_ALL, "");
    initscr();
    raw();
    noecho();
    curs_set(0);
}

Ncurses::~Ncurses() {
    endwin();
}

Ncurses::Screen::Screen(int height, int width, int y, int x) : lines_(height), cols_(width) {
    window_ = newwin(height, width, y, x);
    if (window_ == nullptr) throw std::runtime_error("Failed to create a window");
    keypad(window_, true);
}

Ncurses::Screen::Screen(WINDOW *window) : lines_(LINES - 1), cols_(COLS - 1) {
    window_ = window;
    keypad(window_, true);
}

Ncurses::Screen::~Screen() {
    delwin(window_);
}

int Ncurses::Screen::getCharToLower() const {
    return std::tolower(wgetch(window_));
}

void Ncurses::Screen::putAt(const std::string &string, int y, int x) const {
    wmove(window_, y, 0);
    wclrtoeol(window_);
    wmove(window_, y, x);
    waddstr(window_, string.c_str());
    wrefresh(window_);
}


void Ncurses::Screen::putAt(const std::wstring &string, int y, int x) const {
    wmove(window_, y, 0);
    wclrtoeol(window_);
    wmove(window_, y, x);
#ifdef waddwstr
    waddwstr(window_, string.c_str());
#else
    waddstr(window_, utils::utfToString(string).c_str());
#endif
    wrefresh(window_);
}

void Ncurses::Screen::putFor(const std::string &string, int y, int x, std::chrono::seconds duration) const {
    putAt(string, y, x);
    wrefresh(window_);
    std::this_thread::sleep_for(duration);
    wmove(window_, y, x);
    wclrtoeol(window_);
    wrefresh(window_);
}

void Ncurses::Screen::putFor(const std::wstring &string, int y, int x, std::chrono::seconds duration) const {
    putAt(string, y, x);
    wrefresh(window_);
    std::this_thread::sleep_for(duration);
    wmove(window_, y, x);
    wclrtoeol(window_);
    wrefresh(window_);
}

static std::vector<std::string> getWrappedLines(const std::string &string, int width) {
    std::vector<std::string> lines;
    auto strLen{string.length()};

    for (auto i = 0; i < strLen;) {
        auto charWrappedLine = string.substr(i, width);
        auto lastCharIdx{charWrappedLine.length() - 1};

        // if still other chars in the string and the last char in the substring not '\n' or ' '
        if (i + width < strLen && charWrappedLine[lastCharIdx] != '\n' && charWrappedLine[lastCharIdx] != ' ') {
            auto lastSpace{charWrappedLine.find_last_of(' ')};
            auto wordWrappedLine = charWrappedLine.substr(0, lastSpace);
            i += static_cast<int>(wordWrappedLine.size());
            lines.push_back(std::move(wordWrappedLine));
        } else {
            i += width;
            lines.push_back(std::move(charWrappedLine));
        }
    }

    return lines;
}

static std::vector<std::wstring> getWrappedLines(const std::wstring &string, int width) {
    std::vector<std::wstring> lines;
    auto strLen{string.length()};

    for (auto i = 0; i < strLen;) {
        auto charWrappedLine = string.substr(i, width);
        auto lastCharIdx{charWrappedLine.length() - 1};

        // if still other chars in the string and the last char in the substring not '\n' or ' '
        if (i + width < strLen && charWrappedLine[lastCharIdx] != '\n' && charWrappedLine[lastCharIdx] != ' ') {
            auto lastSpace{charWrappedLine.find_last_of(' ')};
            auto wordWrappedLine = charWrappedLine.substr(0, lastSpace);
            i += static_cast<int>(wordWrappedLine.size());
            lines.push_back(std::move(wordWrappedLine));
        } else {
            i += width;
            lines.push_back(std::move(charWrappedLine));
        }
    }

    return lines;
}

void Ncurses::Screen::putWrapped(const std::string &string, int y, int x, int width) const {
    auto lines{getWrappedLines(string, width)};
    y -= static_cast<int>(lines.size());

    for (auto const &line: lines) {
        putAt(line, ++y, x);
    }

}

void Ncurses::Screen::putWrapped(const std::wstring &string, int y, int x, int width) const {
    auto lines{getWrappedLines(string, width)};
    y -= static_cast<int>(lines.size());

    for (auto const &line: lines) {
        putAt(line, ++y, x);
    }
}

void Ncurses::Screen::putCentered(const std::string &string, int y, int width) const {
    auto lines{getWrappedLines(string, width)};
    y -= static_cast<int>(lines.size());

    for (auto const &line: lines) {
        auto x{cols_ / 2 - static_cast<int>(line.size() / 2)};
        putAt(line, ++y, x);
    }
}

void Ncurses::Screen::putCentered(const std::wstring &string, int y, int width) const {
    auto lines{getWrappedLines(string, width)};
    y -= static_cast<int>(lines.size());

    for (auto const &line: lines) {
        auto x{cols_ / 2 - static_cast<int>(line.size() / 2)};
        putAt(line, ++y, x);
    }
}

void Ncurses::Screen::putCenteredFor(const std::string &string, int y, int width, std::chrono::seconds duration) const {
    auto lines{getWrappedLines(string, width)};
    y -= static_cast<int>(lines.size());

    for (auto const &line: lines) {
        auto x{cols_ / 2 - static_cast<int>(line.size() / 2)};
        putAt(line, ++y, x);
    }
    std::this_thread::sleep_for(duration);
    for (auto i{y - static_cast<int>(lines.size()) + 1}; i < y; ++i) {
        wmove(window_, i, 0);
        wclrtoeol(window_);
    }
    wrefresh(window_);
}

void
Ncurses::Screen::putCenteredFor(const std::wstring &string, int y, int width, std::chrono::seconds duration) const {
    auto lines{getWrappedLines(string, width)};
    y -= static_cast<int>(lines.size());

    for (auto const &line: lines) {
        auto x{cols_ / 2 - static_cast<int>(line.size() / 2)};
        putAt(line, ++y, x);
    }
    std::this_thread::sleep_for(duration);
    for (auto i{y - static_cast<int>(lines.size()) + 1}; i < y; ++i) {
        wmove(window_, i, 0);
        wclrtoeol(window_);
    }
    wrefresh(window_);
}

int Ncurses::Screen::ask(const std::wstring &string, const std::wstring &validChars, unsigned int retries) const {
    int ans;
    while (retries--) {
        putAt(string, 0, 0);
        ans = getCharToLower();
        if (validChars.find(ans) == std::wstring::npos) {
            putFor(std::to_wstring(ans) + std::wstring(L" is not a valid answer"), lines_ / 2, cols_ / 2 - 11,
                   std::chrono::seconds(1));
            ans = '\0';
        }
    }
    return ans;
}

void Ncurses::Screen::clear() {
    wclear(window_);
    wrefresh(window_);
}

int Ncurses::Screen::getLines() const {
    return lines_;
}

int Ncurses::Screen::getCols() const {
    return cols_;
}


void Ncurses::Screen::resize(int lines, int cols) {
    lines_ = lines;
    cols_ = cols;
    wresize(window_, lines, cols);
    wclear(window_);
}
