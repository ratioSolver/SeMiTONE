#pragma once

#ifdef VERBOSE_LOG
#include <iostream>

#ifdef WIN32
#define COLOR_NORMAL ""
#define COLOR_RED ""
#define COLOR_GREEN ""
#define COLOR_YELLOW ""
#else
#define COLOR_NORMAL "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#endif

#define LOG_ERR(msg) std::cerr << COLOR_RED << __FILE__ << "(" << __LINE__ << "): " << msg << COLOR_NORMAL << '\n'
#define LOG_WARN(msg) std::clog << COLOR_YELLOW << __FILE__ << "(" << __LINE__ << "): " << msg << COLOR_NORMAL << '\n'
#define LOG_DEBUG(msg) std::clog << COLOR_GREEN << __FILE__ << "(" << __LINE__ << "): " << msg << COLOR_NORMAL << '\n'
#define LOG(msg) std::cout << COLOR_NORMAL << __FILE__ << "(" << __LINE__ << "): " << msg << COLOR_NORMAL << '\n'
#else
#define LOG_ERR(msg)
#define LOG_WARN(msg)
#define LOG_DEBUG(msg)
#define LOG(msg)
#endif