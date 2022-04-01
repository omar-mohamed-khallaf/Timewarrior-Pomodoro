#include "utils.h"
#include <thread>


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

char utils::Ncurses::Screen::getCharLower() {
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

char utils::Ncurses::Screen::ask(const std::string &string, const std::string &validChars, unsigned int retries) {
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

int utils::Ncurses::Screen::getLines() const {
    return lines_;
}

int utils::Ncurses::Screen::getCols() const {
    return cols_;
}

utils::Ncurses::Screen::Screen(WINDOW *window) : lines_(LINES - 1), cols_(COLS - 1) {
    window_ = window;
    keypad(window_, true);
}

utils::SDL2::SDL2() {
    if (isInitialized_) return;
    if (SDL_Init(SDL_INIT_AUDIO) < 0) throw std::runtime_error("Failed to initialize SDL2");
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        throw std::runtime_error("Failed to initialize SDL2 mixer");
    isInitialized_ = true;
}

utils::SDL2::~SDL2() {
    Mix_Quit();
    SDL_Quit();
    isInitialized_ = false;
}

utils::SDL2::Sample::Sample(const std::string &musicFile) : musicPtr_(Mix_LoadMUS(musicFile.c_str()), Mix_FreeMusic) {
    if (musicPtr_ == nullptr)
        throw std::runtime_error(std::string("Failed to play musicPtr_: ").append(Mix_GetError()));

}

void utils::SDL2::Sample::play() {
    Mix_PlayMusic(musicPtr_.get(), 0);
}
