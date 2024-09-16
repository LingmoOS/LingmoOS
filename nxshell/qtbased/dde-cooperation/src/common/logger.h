// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGGER_H
#define LOGGER_H

#include "logging/config.h"
#include "logging/logger.h"

#include <sstream>

namespace deepin_cross {

enum LogLevel {
    debug = 0,
    info = 1,
    warning = 2,
    error = 3,
    fatal = 4
};

inline LogLevel g_logLevel = warning;

template <int N>
constexpr const char* path_base(const char(&s)[N], int i = N - 1) {
    return (s[i] == '/' || s[i] == '\\') ? (s + i + 1) : (i == 0 ? s : path_base(s, i - 1));
}

template <int N>
constexpr int path_base_len(const char(&s)[N], int i = N - 1) {
    return (s[i] == '/' || s[i] == '\\') ? (N - 2 - i) : (i == 0 ? N-1 : path_base_len(s, i - 1));
}

class LogStream;
class Logger : public CppCommon::Singleton<Logger>
{
    friend CppCommon::Singleton<Logger>;
public:
    Logger();

    ~Logger();

    void init(const std::string &logpath, const std::string &logname);

    LogStream log(const char* fname, unsigned line, int level);

    std::ostringstream& buffer();
    void logout();

private:
    static const char *_levels[];
    CppLogging::Logger _logger;
    std::ostringstream _buffer;
    int _lv;
};

// 代理类 LogStream
class LogStream {
public:
    LogStream(Logger& logger) : _logger(logger) {};

    ~LogStream() {
        // 在构时调用输出
        _logger.logout();
    }

    template<typename T>
    LogStream& operator<<(const T& data) {
        _logger.buffer() << data;
        return *this;
    };

    // 处理 std::endl 和其他操纵符
    LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        manip(_logger.buffer());
        _logger.logout();
        return *this;
    };

private:
    Logger& _logger;
};

}

#endif // LOGGER_H
