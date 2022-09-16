#include <thread>
#include <locale>

#include "Ncurses.h"
#include "utils.h"


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

Ncurses::Screen::~Screen() {
    delwin(window_);
}

auto Ncurses::Screen::getCharToLower() -> int {
    return std::tolower(wgetch(window_));
}

auto Ncurses::Screen::putLineAt(const std::wstring &string, int y, int x) -> void {
    wmove(window_, y, 0);
    wclrtoeol(window_);
    wmove(window_, y, x);
#ifdef waddwstr
    waddwstr(window_, string.c_str());
#else
    waddstr(window_, utils::toString(string).c_str());
#endif
    wrefresh(window_);
}

auto Ncurses::Screen::putLineFor(const std::wstring &string, int y, int x, std::chrono::seconds duration) -> void {
    putLineAt(string, y, x);
    wrefresh(window_);
    std::this_thread::sleep_for(duration);
    wmove(window_, y, x);
    wclrtoeol(window_);
    wrefresh(window_);
}

auto Ncurses::Screen::putLineWrapped(const std::wstring &string, int y, int x, int width) -> void {
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
            i += width;
            putLineAt(charWrappedLine, y + l, x);
        }
    }
}


auto Ncurses::Screen::ask(const std::wstring &string, const std::wstring &validChars, unsigned int retries) -> int {
    int ans;
    while (retries--) {
        putLineAt(string, 0, 0);
        ans = getCharToLower();
        if (validChars.find(ans) == std::wstring::npos) {
            putLineFor(std::to_wstring(ans) + std::wstring(L" is not a valid answer"), lines_ / 2, cols_ / 2 - 11,
                       std::chrono::seconds(1));
            ans = '\0';
        }
    }
    return ans;
}

auto Ncurses::Screen::clear() -> void {
    wclear(window_);
}

auto Ncurses::Screen::getLines() const -> int {
    return lines_;
}

auto Ncurses::Screen::getCols() const -> int {
    return cols_;
}

Ncurses::Screen::Screen(WINDOW *window) : lines_(LINES - 1), cols_(COLS - 1) {
    window_ = window;
    keypad(window_, true);
}

void Ncurses::Screen::resize(int lines, int cols) {
    lines_ = lines;
    cols_ = cols;
    wresize(window_, lines, cols);
    wclear(window_);
}