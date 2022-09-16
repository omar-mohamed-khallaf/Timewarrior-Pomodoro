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

        [[nodiscard]] int getLines() const;

        [[nodiscard]] int getCols() const;

        void putLineAt(const std::wstring &string, int y, int x);

        void putLineFor(const std::wstring &string, int y, int x, std::chrono::seconds duration);

        void putLineWrapped(const std::wstring &string, int y, int x, int width);

        int ask(const std::wstring &string, const std::wstring &validChars, unsigned int retries);

        void clear();

        void resize(int lines, int cols);

    private:
        WINDOW *window_ = stdscr;
        int lines_ = LINES;
        int cols_ = COLS;
    };
};
