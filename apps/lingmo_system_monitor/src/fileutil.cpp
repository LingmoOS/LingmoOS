// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileutil.h"

#include <cerrno>
#include <charconv>
#include <climits>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

std::string readFile(const std::string& path)
{
    const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return {};
    std::string contents;
    char buffer[16384];
    for (;;) {
        const ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            break;
        contents.append(buffer, size_t(count));
    }
    close(fd);
    return contents;
}

static bool isSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

std::vector<std::string_view> splitFields(std::string_view line)
{
    std::vector<std::string_view> fields;
    size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && isSpace(line[i]))
            ++i;
        const size_t start = i;
        while (i < line.size() && !isSpace(line[i]))
            ++i;
        if (i > start)
            fields.push_back(line.substr(start, i - start));
    }
    return fields;
}

std::vector<std::string_view> split(std::string_view text, char separator)
{
    std::vector<std::string_view> pieces;
    size_t start = 0;
    for (;;) {
        const size_t end = text.find(separator, start);
        if (end == std::string_view::npos) {
            pieces.push_back(text.substr(start));
            return pieces;
        }
        pieces.push_back(text.substr(start, end - start));
        start = end + 1;
    }
}

std::string_view trimmed(std::string_view text)
{
    while (!text.empty() && isSpace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && isSpace(text.back()))
        text.remove_suffix(1);
    return text;
}

bool parseNumber(std::string_view text, uint64_t* value)
{
    text = trimmed(text);
    if (text.empty() || text.front() == '-' || text.front() == '+')
        return false;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), *value);
    return error == std::errc() && end == text.data() + text.size();
}

bool parseNumber(std::string_view text, int64_t* value)
{
    text = trimmed(text);
    if (!text.empty() && text.front() == '+')
        text.remove_prefix(1);
    if (text.empty())
        return false;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), *value);
    return error == std::errc() && end == text.data() + text.size();
}

uint64_t toUnsigned(std::string_view text)
{
    uint64_t value = 0;
    return parseNumber(text, &value) ? value : 0;
}

int64_t toSigned(std::string_view text)
{
    int64_t value = 0;
    return parseNumber(text, &value) ? value : 0;
}

bool fileExists(const std::string& path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

std::string symLinkTarget(const std::string& path)
{
    char target[PATH_MAX];
    const ssize_t length = readlink(path.c_str(), target, sizeof(target));
    if (length <= 0 || size_t(length) == sizeof(target))
        return {};
    std::string result(target, size_t(length));
    if (result.front() != '/') {
        const size_t slash = path.rfind('/');
        result = (slash == std::string::npos ? std::string(".") : path.substr(0, slash)) + '/' + result;
        char resolved[PATH_MAX];
        // Relative targets come out clean, as the path of the file they name.
        if (realpath(result.c_str(), resolved))
            result = resolved;
    }
    return result;
}

std::string fileName(std::string_view path)
{
    const size_t slash = path.rfind('/');
    return std::string(slash == std::string_view::npos ? path : path.substr(slash + 1));
}

std::string completeBaseName(std::string_view path)
{
    std::string name = fileName(path);
    const size_t dot = name.rfind('.');
    if (dot != std::string::npos)
        name.resize(dot);
    return name;
}

std::string canonicalPath(const std::string& path)
{
    char resolved[PATH_MAX];
    return realpath(path.c_str(), resolved) ? std::string(resolved) : std::string();
}

std::vector<std::string> directoryEntries(const std::string& path)
{
    std::vector<std::string> entries;
    DIR* directory = opendir(path.c_str());
    if (!directory)
        return entries;
    while (const dirent* entry = readdir(directory)) {
        const std::string_view name = entry->d_name;
        if (name != "." && name != "..")
            entries.emplace_back(name);
    }
    closedir(directory);
    return entries;
}

std::string validUtf8(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        const auto byte = uint8_t(text[i]);
        size_t length = 0;
        uint32_t codePoint = 0;
        if (byte < 0x80) {
            result += char(byte);
            ++i;
            continue;
        } else if ((byte & 0xE0) == 0xC0) {
            length = 2;
            codePoint = byte & 0x1F;
        } else if ((byte & 0xF0) == 0xE0) {
            length = 3;
            codePoint = byte & 0x0F;
        } else if ((byte & 0xF8) == 0xF0) {
            length = 4;
            codePoint = byte & 0x07;
        }
        bool valid = length > 0 && i + length <= text.size();
        for (size_t k = 1; valid && k < length; ++k) {
            const auto next = uint8_t(text[i + k]);
            valid = (next & 0xC0) == 0x80;
            codePoint = codePoint << 6 | (next & 0x3F);
        }
        // Overlong forms, surrogates and values past Unicode are invalid too.
        static constexpr uint32_t kMinimum[] = {0, 0, 0x80, 0x800, 0x10000};
        valid = valid && codePoint >= kMinimum[length] && codePoint <= 0x10FFFF
                && (codePoint < 0xD800 || codePoint > 0xDFFF);
        if (valid) {
            result.append(text.substr(i, length));
            i += length;
        } else {
            result += "\xEF\xBF\xBD";
            ++i;
        }
    }
    return result;
}

uint64_t counterDelta(uint64_t current, uint64_t previous)
{
    return current >= previous ? current - previous : 0;
}

double byteRate(uint64_t current, uint64_t previous, double elapsed)
{
    return elapsed > 0 ? counterDelta(current, previous) / elapsed : 0;
}
