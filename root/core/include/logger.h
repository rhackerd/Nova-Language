#pragma once
#include <fmt/core.h>
#include <termcolor/termcolor.hpp>
#include <iostream>
#include <sstream>

// ==================== Logging Macros ====================
static int logLinesPrinted = 0;

inline int getPrintedLines() {
    return logLinesPrinted;
}

inline void resetPrintedLines() {
    logLinesPrinted = 0;
}

template<typename... Args>
void NCINFO(fmt::format_string<Args...> fmt, Args&&... args) {
    std::cout << termcolor::on_green << termcolor::bold
              << " INFO " << termcolor::reset << " "
              << fmt::format(fmt, std::forward<Args>(args)...) << "\n";
    logLinesPrinted++;
}

inline void _ncinfo() {
    std::cout << termcolor::on_green << termcolor::bold << " INFO " << termcolor::reset << std::flush;
}


template<typename... Args>
void NCWARN(fmt::format_string<Args...> fmt, Args&&... args) {
    std::cout << termcolor::on_yellow << termcolor::bold
              << " WARN " << termcolor::reset << " "
              << fmt::format(fmt, std::forward<Args>(args)...) << "\n";
    logLinesPrinted++;
}

inline void _ncwarn() {
    std::cout << termcolor::on_yellow << termcolor::bold << " WARN " << termcolor::reset << std::flush;
}

template<typename... Args>
void NCERROR(fmt::format_string<Args...> fmt, Args&&... args) {
    std::cerr << termcolor::on_red << termcolor::bold
              << " EROR " << termcolor::reset << " "
              << fmt::format(fmt, std::forward<Args>(args)...) << std::endl << std::flush;
    logLinesPrinted++;
}

inline void _nceror() {
    std::cout << termcolor::on_red << termcolor::bold << " EROR " << termcolor::reset << std::flush;
}