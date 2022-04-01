#include "utils.h"
#include <thread>

char utils::Ncurses::Screen::getCharLower() {
    return static_cast<char>(std::tolower(wgetch(window_)));
}

void utils::Ncurses::Screen::putStringAtLine(const std::string &string, int y, int x) {
    wmove(window_, y, 0);
    wclrtoeol(window_);
    wmove(window_, y, x);
    waddstr(window_, string.c_str());
}

char utils::Ncurses::Screen::ask(const std::string &string, const std::string &validChars, unsigned int retries) {
    char ans;
    while (retries--) {
        putStringAtLine(string, 0, 0);
        ans = getCharLower();
        if (validChars.find(ans) == std::string::npos) {
            putStringAtLine(std::string(" is not a valid answer").insert(0, 1, ans), LINES / 2, COLS / 2 - 11);
            std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1, 1>>(1));
            wclrtoeol(window_);
            ans = '\0';
        }
    }
    return ans;
}


void utils::Ncurses::Screen::clearScreen() {
    wclear(window_);
}

void utils::Ncurses::Screen::render() {
    wrefresh(window_);
}

int utils::Ncurses::Screen::getLines() const {
    return lines_;
}

int utils::Ncurses::Screen::getCols() const {
    return cols_;
}
