// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Feeds a seeded random stream of text, controls and escape sequences into the
// emulation and prints a hash of the whole state after every chunk, so two
// builds can be compared line by line. The stream, the chunking and the hash
// are those of the earlier build's comparison tool, so their output matches.
#include <clocale>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "base/event_loop.h"

#include "History.h"
#include "Screen.h"
#include "ScreenWindow.h"
#include "Vt102Emulation.h"
#include "md5.h"

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

static std::string number(int value)
{
    return std::to_string(value);
}

static std::string piece(Random& rng)
{
    auto num = [&](int max) { return number(rng.bounded(max)); };
    const int kind = rng.bounded(100);
    std::string out;
    if (kind < 40) {
        const int n = 1 + rng.bounded(120);
        for (int i = 0; i < n; ++i)
            out += char(32 + rng.bounded(95));
    } else if (kind < 50) {
        static const char* texts[] = {"\xe4\xb8\xad\xe6\x96\x87", "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e",
                                      "e\xcc\x81", "\xf0\x9f\x98\x80", "\xf0\x9f\x87\xa9\xf0\x9f\x87\xaa",
                                      "\xe2\x80\x8b", "\xee\x80\x80", "\xc2\x9b" "2J", "\xff", "\x80", "\xe4\xb8",
                                      "\xc3\xa9", "\xcc\x81", "\xef\xbf\xbd", "\xe2\x94\x80\xe2\x94\x82"};
        out += texts[rng.bounded(int(std::size(texts)))];
    } else if (kind < 60) {
        static const char controls[] = {'\r', '\n', '\t', '\b', '\a', '\x0b', '\x0c', '\x0e', '\x0f', '\x7f',
                                        '\x18', '\x1a', '\0', '\x05'};
        out += controls[rng.bounded(int(sizeof(controls)))];
        if (rng.bounded(2))
            out += "\r\n";
    } else if (kind < 70) {
        static const char* escapes[] = {"\x1b" "7", "\x1b" "8", "\x1b" "D", "\x1b" "M", "\x1b" "E", "\x1b(0",
                                        "\x1b(B", "\x1b)0", "\x1b#8", "\x1b=", "\x1b>", "\x1b" "H", "\x1b(A"};
        out += escapes[rng.bounded(int(std::size(escapes)))];
    } else if (kind < 92) {
        static const char finals[] = "ABCDHfGdJKLM@PXSTrmhlnc";
        const char final = finals[rng.bounded(int(sizeof(finals) - 1))];
        out += "\x1b[";
        if (final == 'h' || final == 'l') {
            static const char* modes[] = {"4", "?7", "?6", "?25", "?1049", "?2", "?1", "20", "?1047", "?3"};
            out += modes[rng.bounded(int(std::size(modes)))];
        } else if (final == 'm') {
            static const char* sgr[] = {"0", "1", "4", "7", "31", "42", "38;5;", "48;2;", "4:3", "38:2::", "22",
                                        "39", "49", "5", "9"};
            const int count = 1 + rng.bounded(4);
            for (int i = 0; i < count; ++i) {
                if (i)
                    out += ';';
                std::string p = sgr[rng.bounded(int(std::size(sgr)))];
                out += p;
                auto endsWith = [&](const char* suffix) {
                    const size_t length = std::strlen(suffix);
                    return p.size() >= length && p.compare(p.size() - length, length, suffix) == 0;
                };
                if (endsWith(";5;")) {
                    out += num(256);
                } else if (endsWith(";2;")) {
                    // The original's chains of temporaries draw their numbers right to left.
                    const std::string b = num(256);
                    const std::string g = num(256);
                    const std::string r = num(256);
                    out += r + ";" + g + ";" + b;
                } else if (endsWith("::")) {
                    const std::string b = num(256);
                    const std::string g = num(256);
                    const std::string r = num(256);
                    out += r + ":" + g + ":" + b;
                }
            }
        } else if (final == 'r') {
            if (rng.bounded(3)) {
                const std::string bottom = num(60);
                const std::string top = num(30);
                out += top + ";" + bottom;
            }
        } else if (final == 'n') {
            out += rng.bounded(2) ? "6" : "5";
        } else if (rng.bounded(3)) {
            out += num(rng.bounded(2) ? 5 : 80);
            if (final == 'H' || final == 'f')
                out += ";" + num(200);
        }
        out += final;
    } else if (kind < 95) {
        const char* end = rng.bounded(2) ? "\a" : "\x1b\\";
        const std::string value = num(1000);
        const std::string title = num(3);
        out += "\x1b]" + title + ";title " + value + end;
    } else if (kind < 98) {
        // VT52 sequences, meaningful once ?2l switched to VT52.
        static const char* vt52[] = {"\x1b" "A", "\x1b" "B", "\x1b" "C", "\x1b" "D", "\x1b<", "\x1b" "F",
                                     "\x1b" "G", "\x1bZ"};
        out += vt52[rng.bounded(int(std::size(vt52)))];
        if (rng.bounded(3) == 0) {
            const char column = char(32 + rng.bounded(90));
            const char row = char(32 + rng.bounded(40));
            out += "\x1bY";
            out += row;
            out += column;
        }
    } else {
        out += "\x1b[?2l";
    }
    return out;
}

static void addColor(Md5& hash, const CharacterColor& color)
{
    char bytes[sizeof(CharacterColor)];
    std::memcpy(bytes, &color, sizeof(bytes));
    hash.addData(bytes, sizeof(bytes));
}

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");
    Lingmo::base::EventLoop loop;

    const uint32_t seed = argc > 1 ? uint32_t(std::stoul(argv[1])) : 1;
    const int chunks = argc > 2 ? std::stoi(argv[2]) : 20000;

    Vt102Emulation emulation;
    ScreenWindow* window = emulation.createWindow();
    emulation.setKeyBindings(std::string());
    if (argc > 3 && std::string(argv[3]) == "file")
        emulation.setHistory(HistoryTypeFile());
    else
        emulation.setHistory(HistoryTypeBuffer(500));
    emulation.setImageSize(24, 80);
    std::string sent;
    auto subscription = emulation.sendData.connect([&](const char* data, int length) {
        sent.append(data, size_t(length));
    });

    Random rng(seed);
    std::string pending;
    std::vector<Character> image;
    for (int chunk = 0; chunk < chunks; ++chunk) {
        while (pending.size() < 8192)
            pending += piece(rng);
        const int length = 1 + rng.bounded(rng.bounded(4) ? 64 : 4096);
        emulation.receiveData(pending.data(), length);
        pending.erase(0, size_t(length));

        if (rng.bounded(500) == 0) {
            // The original's arguments are drawn right to left.
            const int columns = 2 + rng.bounded(200);
            const int lines = 2 + rng.bounded(60);
            emulation.setImageSize(lines, columns);
        }
        if (chunk % 50 == 0)
            loop.dispatch();

        const Screen* screen = window->screen();
        const int lines = screen->getHistLines() + screen->getLines();
        image.assign(size_t(lines * screen->getColumns()), Character());
        screen->getImage(image.data(), int(image.size()), 0, lines - 1);
        Md5 hash;
        for (const Character& c : image) {
            const uint32_t ch = uint32_t(c.character);
            hash.addData(&ch, 4);
            hash.addData(&c.rendition, 1);
            addColor(hash, c.foregroundColor);
            addColor(hash, c.backgroundColor);
        }
        const auto properties = screen->getLineProperties(0, lines - 1);
        hash.addData(properties.data(), properties.size());
        const std::string state = number(screen->getCursorX()) + "," + number(screen->getCursorY()) + ","
                + number(lines) + "," + number(screen->getColumns()) + "," + sent;
        hash.addData(state);
        std::printf("%d %s\n", chunk, hash.hexResult().c_str());
    }
    return 0;
}
