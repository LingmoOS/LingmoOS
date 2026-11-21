// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Feeds program output into the terminal emulation in pty-sized chunks and
// reports throughput; the display is not involved. The workloads are those of
// the earlier build's benchmark.
#include <chrono>
#include <clocale>
#include <cstdio>
#include <malloc.h>
#include <random>
#include <string>
#include <vector>

#include "base/elapsed_timer.h"
#include "base/event_loop.h"

#include "History.h"
#include "ScreenWindow.h"
#include "Vt102Emulation.h"

using namespace Konsole;

// A Mersenne twister on a seed_seq of one value: the same stream on every run.
class Random
{
public:
    explicit Random(uint32_t seed)
    {
        std::seed_seq sequence{seed};
        m_engine.seed(sequence);
    }

    int bounded(int highest)
    {
        uint64_t value = m_engine();
        value *= uint32_t(highest);
        value /= uint64_t(0xffffffff) + 1;
        return int(value);
    }

private:
    std::mt19937 m_engine;
};

static std::string workload(const std::string& kind, size_t bytes)
{
    std::string out;
    out.reserve(bytes + 4096);
    Random rng(42);
    static const char* words[] = {"error", "warning", "build", "src/main.cpp", "Linking", "CXX",
                                  "object", "the", "a", "function", "return", "value", "int",
                                  "QString", "const", "auto", "[100%]", "make[2]:", "-O2", "-Wall"};
    int line = 0;
    while (out.size() < bytes) {
        if (kind == "plain" || kind == "color") {
            const int count = 3 + rng.bounded(14);
            for (int i = 0; i < count; ++i) {
                const char* word = words[rng.bounded(20)];
                if (kind == "color" && rng.bounded(4) == 0) {
                    out += "\x1b[1;3" + std::to_string(1 + rng.bounded(6)) + "m";
                    out += word;
                    out += "\x1b[0m";
                } else {
                    out += word;
                }
                out += ' ';
            }
            out += "\r\n";
        } else if (kind == "utf8") {
            // CJK, Japanese, accented Latin, a dash, a check mark and Hangul, as the original line.
            out += "\xe7\xac\xac" + std::to_string(line)
                    + "\xe8\xa1\x8c \xe4\xb8\xad\xe6\x96\x87\xe8\xbe\x93\xe5\x87\xba "
                      "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e\xe3\x81\xae\xe3\x83\x86\xe3\x82\xad\xe3\x82\xb9\xe3\x83\x88 "
                      "\xc3\x9cn\xc3\xaf" "c\xc3\xb6" "d\xc3\xa9 \xe2\x80\x94 r\xc3\xa9sum\xc3\xa9 \xe2\x9c\x93 "
                      "\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4\r\n";
        } else if (kind == "short") {
            // seq-style output: a short line per number.
            out += std::to_string(line) + "\r\n";
        } else if (kind == "redraw") {
            // A full-screen program repainting every row in place, without scrolling.
            const int row = line % 52;
            out += "\x1b[" + std::to_string(row + 1) + ";1H\x1b[3" + std::to_string(row % 8) + "m";
            for (int i = 0; i < 170; ++i)
                out += char('a' + (line + i) % 26);
            out += "\x1b[0m";
        } else if (kind == "long") {
            for (int i = 0; i < 1500; ++i)
                out += char('a' + rng.bounded(26));
            out += "\r\n";
        }
        ++line;
    }
    return out;
}

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");
    Lingmo::base::EventLoop loop;
    std::vector<std::string> kinds;
    for (int i = 1; i < argc; ++i)
        kinds.push_back(argv[i]);
    if (kinds.empty())
        kinds = {"plain", "color", "utf8", "long", "short", "redraw"};

    for (const std::string& kind : kinds) {
        const std::string data = workload(kind, 32 * 1024 * 1024);
        Vt102Emulation emulation;
        emulation.createWindow();
        emulation.setKeyBindings(std::string());
        emulation.setHistory(HistoryTypeBuffer(10000));
        emulation.setImageSize(52, 175);

        Lingmo::base::ElapsedTimer timer, frame;
        timer.start();
        frame.start();
        for (size_t offset = 0; offset < data.size(); offset += 4096) {
            emulation.receiveData(data.data() + offset, int(std::min<size_t>(4096, data.size() - offset)));
            // The pty and the display timers share one thread.
            if (frame.elapsedMs() >= 16) {
                loop.dispatch();
                frame.restart();
            }
        }
        loop.dispatch();
        const double ms = double(timer.elapsed().count()) / 1e6;
        const size_t heap = mallinfo2().uordblks;
        std::printf("%-6s %6.1f MB in %7.1f ms  %6.1f MB/s  heap in use %.1f MB\n", kind.c_str(),
                    double(data.size()) / 1048576.0, ms, double(data.size()) / 1048576.0 / (ms / 1000),
                    double(heap) / 1048576.0);
        std::fflush(stdout);
    }
    return 0;
}
