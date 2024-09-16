// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <memory>
#include <vector>
#include <fstream>
#include <locale>
#include <iomanip>

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

namespace util {

#define MAX_BACKTRACE_FRAMES 128
#define BUF_LEN 256

// Custom deleter for free system call
struct Free {
    void operator()(char **p)
    {
        free(p);
    }
};
// unique_ptr type define for backtrace symbols return data structure
using BacktraceSymbols = std::unique_ptr<char *, Free>;

/**
 * @brief Print stack trace when specified signal caught
 * @param signum Signal that caused crash
 */
static inline void printStacktrace(int signum)
{
    // timestamp buffer
    char ts[BUF_LEN] {};
    // current timestamp
    std::time_t tm = std::time(nullptr);
    // format current local time as standard iso format string
    auto tslen = std::strftime(ts, BUF_LEN, "%FT%T", std::localtime(&tm));
    ts[tslen] = 0;
    // log filename buffer
    char logstr[BUF_LEN] {};
    // format log filename
    auto len = snprintf(logstr, BUF_LEN, "deepin-system-monitor.%s.crash", ts);
    logstr[len] = 0;

    // open log output stream
    std::string logN {"/tmp/"};
    logN.append(logstr);
    std::ofstream log(logN, std::ios::out);

    // get backtrace symbols list
    //    std::vector<void *> stack(MAX_BACKTRACE_FRAMES);
    //    auto size = backtrace(stack.data(), int(stack.size()));
    //    BacktraceSymbols symbols(backtrace_symbols(stack.data(), size));
    void *stack[MAX_BACKTRACE_FRAMES];
    auto size = backtrace(stack, MAX_BACKTRACE_FRAMES);
    BacktraceSymbols symbols(backtrace_symbols(stack, size));

    // print all symbols as formatted text to output log stream
    if (symbols && size > 0) {
        log << "Stack trace:" << std::endl;

        for (int i = 0; i < size; i++) {
            log << '\t' << "#" << std::left << std::setw(4) << i << symbols.get()[i] << std::endl;
        }
    }

    // reset signal handler to default
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SIG_DFL;
    sigaction(signum, &act, nullptr);
    // raise origin signal
    raise(signum);
}

/**
 * @brief Install crash handler for application ifself
 */
void installCrashHandler()
{
    // action to take when signal get caught
    struct sigaction act {
    };
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = printStacktrace;

    // Invalid memory reference
    sigaction(SIGSEGV, &act, nullptr);
    // Floating-point exception
    sigaction(SIGFPE, &act, nullptr);
    // Illegal Instruction
    sigaction(SIGILL, &act, nullptr);
    // Abort signal from abort call
    sigaction(SIGABRT, &act, nullptr);
}

} // namespace util

#endif // STACK_TRACE_H
