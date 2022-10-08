#pragma once

#include <ncurses.h>
#include <string>
#include <chrono>

#define PUT_CENTERED(screen, string, line) screen.putCentered(string, line, sizeof(string))
#define PUT_CENTERED_FOR(screen, string, line, duration) screen.putCenteredFor(string, line, sizeof(string), duration)

class Ncurses {
public:
    explicit Ncurses();

    ~Ncurses();

    class Screen {
    public:
        explicit Screen(WINDOW *window) noexcept(false);

        Screen(int height, int width, int y, int x) noexcept(false);

        ~Screen();

        /**
         * Get the char pressed on keyboard always in lower case
         * @return the char pressed
         */
        int getCharToLower() const;

        /**
         * Get the number of lines of this screen
         * @return the number of lines
         */
        [[nodiscard]] int getLines() const;

        /**
         * Get the number of columns of this screen
         * @return the number of columns
         */
        [[nodiscard]] int getCols() const;

        /**
         * Puts a line at y, x coordinates on a screen
         * @note if string can't fit on the line it's trimmed
         * @param string the string to be printed
         * @param y the line index
         * @param x the column index
         */
        void putAt(const std::string &string, int y, int x) const;

        /**
         * Puts a line at y, x coordinates on a screen
         * @note if string can't fit on the line it's trimmed
         * @param string the string to be printed
         * @param y the line index
         * @param x the column index
         */
        void putAt(const std::wstring &string, int y, int x) const;

        /**
         * Puts a line at y, x coordinates on a screen for a specific duration of time
         * @note suspends the thread that calls the function
         * @note if string can't fit on the line it's trimmed
         * @param string the string to be printed
         * @param y the line index
         * @param x the colum index
         * @param duration the duration of display
         */
        void putFor(const std::string &string, int y, int x, std::chrono::seconds duration) const;

        /**
         * Puts a line at y, x coordinates on a screen for a specific duration of time
         * @note suspends the thread that calls the function
         * @note if string can't fit on the line it's trimmed
         * @param string to be printed
         * @param y the line index
         * @param x the colum index
         * @param duration the duration of display
         */
        void putFor(const std::wstring &string, int y, int x, std::chrono::seconds duration) const;

        /**
         * Puts a line at y, x coordinates on a screen if the line fits in the specified width, otherwise the line is
         * wrapped such that the last word will be on the line at y index
         * @param string the string to be printed
         * @param y the line index
         * @param x the column index
         * @param width the maximum width to wrap after
         */
        void putWrapped(const std::string &string, int y, int x, int width) const;

        /**
         * Puts a line at y, x coordinates on a screen if the line fits in the specified width, otherwise the line is
         * wrapped such that the last word will be on the line at y index
         * @param string the string to be printed
         * @param y the line index
         * @param x the column index
         * @param width the maximum width to wrap after
         */
        void putWrapped(const std::wstring &string, int y, int x, int width) const;

        /**
         * Puts a line centered at y coordinate on a screen if the line fits in the specified width, otherwise the line
         * is wrapped such that the last word will be on the line at y index
         * @param string the string to be printed
         * @param y the line index
         * @param width the maximum width to wrap after
         */
        void putCentered(const std::string &string, int y, int width) const;

        /**
         * Puts a line centered at y coordinate on a screen if the line fits in the specified width, otherwise the line
         * is wrapped such that the last word will be on the line at y index
         * @param string the string to be printed
         * @param y the line index
         * @param width the maximum width to wrap after
         */
        void putCentered(const std::wstring &string, int y, int width) const;

        /**
         * Puts a line for a specific duration centered at y coordinate on a screen if the line fits in the specified
         * width, otherwise the line is wrapped such that the last word will be on the line at y index
         * @param string the string to be printed
         * @param y the line index
         * @param width the maximum width to wrap after
         * @param duration the duration of display
         */
        void putCenteredFor(const std::string &string, int y, int width, std::chrono::seconds duration) const;

        /**
         * Puts a line for a specific duration centered at y coordinate on a screen if the line fits in the specified
         * width, otherwise the line is wrapped such that the last word will be on the line at y index
         * @param string the string to be printed
         * @param y the line index
         * @param width the maximum width to wrap after
         * @param duration the duration of display
         */
        void putCenteredFor(const std::wstring &string, int y, int width, std::chrono::seconds duration) const;

        /**
         * Ask the user a question and get the pressed key back
         * @param string question to show the user
         * @param validChars allowed key presses as responses
         * @param retries number of retries if invalid key is pressed
         * @return the pressed key as an answer
         */
        int ask(const std::wstring &string, const std::wstring &validChars, unsigned int retries) const;

        /**
         * Clears the screen
         */
        void clear();

        /**
         * Updates the screen to use the new number of lines and cols
         * @param lines the number of lines
         * @param cols the number of columns
         */
        void resize(int lines, int cols);

    private:
        WINDOW *window_ = stdscr;
        int lines_ = LINES;
        int cols_ = COLS;
    };
};
