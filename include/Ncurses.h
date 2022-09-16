#pragma once

#include <ncurses.h>
#include <string>
#include <chrono>

class Ncurses {
public:
    explicit Ncurses();

    ~Ncurses();

    class Screen {
    public:
        explicit Screen(WINDOW *window) noexcept(false);

        Screen(int height, int width, int y, int x) noexcept(false);

        ~Screen();

        int getCharToLower();

        int getLines() const;

        int getCols() const;

        void putLineAt(const std::string &string, int y, int x);

        void putLineFor(const std::string &string, int y, int x, std::chrono::seconds duration);

        void putLineWrapped(const std::string &string, int y, int x, int width);

        char ask(const std::string &string, const std::string &validChars, unsigned int retries);

        void clear();

        void resize(int lines, int cols);

    private:
        WINDOW *window_ = stdscr;
        int lines_ = LINES;
        int cols_ = COLS;
    };
};
