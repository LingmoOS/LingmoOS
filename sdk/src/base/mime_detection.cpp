// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Type detection on the shared-mime-info files update-mime-database writes:
// globs2, magic, icons and generic-icons.

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <fnmatch.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

#include "base/file.h"
#include "base/mime_database.h"
#include "base/paths.h"

namespace Lingmo::base::mime {

// How much of a file the magic rules see.
static constexpr size_t kSniffBytes = 16384;

namespace {

struct Glob
{
    int weight = 50;
    std::string type;
    std::string pattern;
    bool caseSensitive = false;
};

struct MagicRule
{
    int indent = 0;
    size_t offset = 0;
    std::string value;
    std::string mask;
    size_t range = 1;
    std::vector<MagicRule> children;
};

struct MagicSection
{
    int priority = 50;
    std::string type;
    std::vector<MagicRule> rules;
};

struct Detection
{
    std::vector<Glob> globs;
    std::vector<MagicSection> magic;
    std::unordered_map<std::string, std::string> icons;
    std::unordered_map<std::string, std::string> genericIcons;
};

} // namespace

static std::vector<std::string> mimeDirectories()
{
    // The user's first, so they override the system's.
    std::vector<std::string> directories = {paths::dataHome() + "/mime"};
    for (const std::string& directory : paths::dataDirectories())
        directories.push_back(directory + "/mime");
    return directories;
}

static void readGlobs(const std::string& path, std::vector<Glob>& globs)
{
    const std::optional<std::string> contents = file::read(path);
    if (!contents)
        return;
    std::istringstream lines(*contents);
    std::string line;
    while (std::getline(lines, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        // weight:type:pattern[:flags]
        const size_t first = line.find(':');
        const size_t second = first == std::string::npos ? first : line.find(':', first + 1);
        if (second == std::string::npos)
            continue;
        Glob glob;
        glob.weight = std::atoi(line.substr(0, first).c_str());
        glob.type = line.substr(first + 1, second - first - 1);
        std::string rest = line.substr(second + 1);
        const size_t flags = rest.find(':');
        if (flags != std::string::npos) {
            glob.caseSensitive = rest.find("cs", flags) != std::string::npos;
            rest.resize(flags);
        }
        glob.pattern = std::move(rest);
        if (glob.pattern != "__NOGLOBS__")
            globs.push_back(std::move(glob));
    }
}

static size_t readNumber(std::string_view data, size_t& i)
{
    size_t value = 0;
    while (i < data.size() && std::isdigit(static_cast<unsigned char>(data[i])))
        value = value * 10 + size_t(data[i++] - '0');
    return value;
}

// One "[indent]>offset=<length><value>[&mask][~word][+range]\n" line.
static bool readMagicRule(std::string_view data, size_t& i, MagicRule& rule)
{
    rule.indent = int(readNumber(data, i));
    if (i >= data.size() || data[i] != '>')
        return false;
    ++i;
    rule.offset = readNumber(data, i);
    if (i + 3 > data.size() || data[i] != '=')
        return false;
    const size_t length = (size_t(static_cast<unsigned char>(data[i + 1])) << 8) | static_cast<unsigned char>(data[i + 2]);
    i += 3;
    if (i + length > data.size())
        return false;
    rule.value = std::string(data.substr(i, length));
    i += length;
    size_t wordSize = 1;
    while (i < data.size() && data[i] != '\n') {
        const char marker = data[i++];
        if (marker == '&') {
            if (i + length > data.size())
                return false;
            rule.mask = std::string(data.substr(i, length));
            i += length;
        } else if (marker == '~') {
            wordSize = readNumber(data, i);
        } else if (marker == '+') {
            rule.range = std::max<size_t>(1, readNumber(data, i));
        } else {
            // An extension this reader does not know: the line is skipped.
            while (i < data.size() && data[i] != '\n')
                ++i;
            if (i < data.size())
                ++i;
            return false;
        }
    }
    if (i < data.size())
        ++i;
    // Values of host-order words are written big-endian.
    if ((wordSize == 2 || wordSize == 4) && length % wordSize == 0 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) {
        for (std::string* bytes : {&rule.value, &rule.mask}) {
            for (size_t start = 0; start + wordSize <= bytes->size(); start += wordSize)
                std::reverse(bytes->begin() + long(start), bytes->begin() + long(start + wordSize));
        }
    }
    return true;
}

static void readMagic(const std::string& path, std::vector<MagicSection>& sections)
{
    const std::optional<std::string> contents = file::read(path);
    static constexpr std::string_view kHeader("MIME-Magic\0\n", 12);
    if (!contents || !std::string_view(*contents).starts_with(kHeader))
        return;
    const std::string_view data = *contents;
    size_t i = kHeader.size();
    MagicSection* section = nullptr;
    // The rules of the section in file order, with their indents.
    std::vector<MagicRule> flat;
    const auto finish = [&flat, &section] {
        if (!section)
            return;
        // Children follow their parent one indent deeper.
        std::vector<MagicRule*> stack;
        for (MagicRule& rule : flat) {
            while (!stack.empty() && stack.back()->indent >= rule.indent)
                stack.pop_back();
            std::vector<MagicRule>& siblings = stack.empty() ? section->rules : stack.back()->children;
            siblings.push_back(std::move(rule));
            stack.push_back(&siblings.back());
        }
        flat.clear();
    };
    while (i < data.size()) {
        if (data[i] == '[') {
            finish();
            const size_t end = data.find("]\n", i);
            if (end == std::string_view::npos)
                return;
            const std::string_view header = data.substr(i + 1, end - i - 1);
            const size_t colon = header.find(':');
            sections.push_back({});
            section = &sections.back();
            section->priority = std::atoi(std::string(header.substr(0, colon)).c_str());
            section->type = colon == std::string_view::npos ? std::string() : std::string(header.substr(colon + 1));
            i = end + 2;
            continue;
        }
        MagicRule rule;
        const size_t start = i;
        if (readMagicRule(data, i, rule)) {
            flat.push_back(std::move(rule));
        } else if (i == start || data[i - 1] != '\n') {
            // Malformed: on to the next line.
            while (i < data.size() && data[i] != '\n')
                ++i;
            ++i;
        }
    }
    finish();
}

static void readPairs(const std::string& path, std::unordered_map<std::string, std::string>& map)
{
    const std::optional<std::string> contents = file::read(path);
    if (!contents)
        return;
    std::istringstream lines(*contents);
    std::string line;
    while (std::getline(lines, line)) {
        const size_t colon = line.find(':');
        if (line.empty() || line[0] == '#' || colon == std::string::npos)
            continue;
        map.emplace(line.substr(0, colon), line.substr(colon + 1));
    }
}

static const Detection& detection()
{
    static const Detection detection = [] {
        Detection result;
        for (const std::string& directory : mimeDirectories()) {
            readGlobs(directory + "/globs2", result.globs);
            readMagic(directory + "/magic", result.magic);
            readPairs(directory + "/icons", result.icons);
            readPairs(directory + "/generic-icons", result.genericIcons);
        }
        return result;
    }();
    return detection;
}

static bool matches(const MagicRule& rule, std::string_view data)
{
    const size_t length = rule.value.size();
    bool found = false;
    for (size_t offset = rule.offset; offset < rule.offset + rule.range && offset + length <= data.size(); ++offset) {
        bool equal = true;
        for (size_t i = 0; i < length && equal; ++i) {
            const auto byte = static_cast<unsigned char>(data[offset + i]);
            const auto value = static_cast<unsigned char>(rule.value[i]);
            const auto mask = rule.mask.empty() ? 0xffu : static_cast<unsigned char>(rule.mask[i]);
            equal = (byte & mask) == (value & mask);
        }
        if (equal) {
            found = true;
            break;
        }
    }
    if (!found)
        return false;
    if (rule.children.empty())
        return true;
    return std::any_of(rule.children.begin(), rule.children.end(),
                       [data](const MagicRule& child) { return matches(child, data); });
}

// The best magic match and its priority, 0 for none.
static std::string sniff(std::string_view data, int* accuracy)
{
    std::string best;
    *accuracy = 0;
    for (const MagicSection& section : detection().magic) {
        if (section.priority <= *accuracy)
            continue;
        if (std::any_of(section.rules.begin(), section.rules.end(),
                        [data](const MagicRule& rule) { return matches(rule, data); })) {
            best = section.type;
            *accuracy = section.priority;
        }
    }
    return best;
}

// Text: a UTF-16 byte order mark, or no control characters but tab, line feed
// and carriage return in the first 128 bytes.
static bool looksLikeText(std::string_view data)
{
    if (data.starts_with("\xfe\xff") || data.starts_with("\xff\xfe"))
        return true;
    for (const char c : data.substr(0, 128)) {
        const auto byte = static_cast<unsigned char>(c);
        if (byte < 32 && byte != 9 && byte != 10 && byte != 13)
            return false;
    }
    return true;
}

namespace {

// The glob matches of one name.
struct GlobMatches
{
    int weight = 0;
    size_t length = 0;
    // Of the highest weight, then the longest pattern.
    std::vector<std::string> best;
    std::vector<std::string> all;

    void add(const Glob& glob)
    {
        if (std::find(all.begin(), all.end(), glob.type) == all.end())
            all.push_back(glob.type);
        if (glob.weight < weight)
            return;
        if (glob.weight > weight) {
            best.clear();
            length = 0;
            weight = glob.weight;
        }
        if (glob.pattern.size() < length)
            return;
        if (glob.pattern.size() > length) {
            best.clear();
            length = glob.pattern.size();
        }
        if (std::find(best.begin(), best.end(), glob.type) == best.end())
            best.push_back(glob.type);
    }
};

} // namespace

static std::string lowered(std::string_view text)
{
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    return result;
}

static GlobMatches matchGlobs(std::string_view fileName)
{
    GlobMatches result;
    const std::string name(fileName);
    const std::string lower = lowered(fileName);
    for (const Glob& glob : detection().globs) {
        const std::string& subject = glob.caseSensitive ? name : lower;
        const bool hit = glob.pattern.find_first_of("*?[") == std::string::npos
                ? subject == glob.pattern
                : fnmatch(glob.pattern.c_str(), subject.c_str(), 0) == 0;
        if (hit)
            result.add(glob);
    }
    return result;
}

bool inherits(std::string_view type, std::string_view ancestor)
{
    const std::string target = canonicalName(ancestor).empty() ? std::string(ancestor) : canonicalName(ancestor);
    std::vector<std::string> pending = {canonicalName(type).empty() ? std::string(type) : canonicalName(type)};
    std::vector<std::string> seen;
    while (!pending.empty()) {
        const std::string current = pending.back();
        pending.pop_back();
        if (current == target)
            return true;
        if (std::find(seen.begin(), seen.end(), current) != seen.end())
            continue;
        seen.push_back(current);
        for (const std::string& parent : parents(current))
            pending.push_back(parent);
    }
    return false;
}

std::string typeForData(std::string_view data)
{
    if (data.empty())
        return "application/x-zerosize";
    int accuracy = 0;
    const std::string sniffed = sniff(data, &accuracy);
    if (accuracy > 0 && !sniffed.empty())
        return sniffed;
    return looksLikeText(data) ? "text/plain" : "application/octet-stream";
}

std::string typeForFileName(std::string_view fileName)
{
    GlobMatches matches = matchGlobs(fileName);
    if (matches.best.empty())
        return "application/octet-stream";
    std::sort(matches.best.begin(), matches.best.end());
    return matches.best.front();
}

std::string typeForFile(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        if (S_ISDIR(info.st_mode))
            return "inode/directory";
        if (S_ISCHR(info.st_mode))
            return "inode/chardevice";
        if (S_ISBLK(info.st_mode))
            return "inode/blockdevice";
        if (S_ISFIFO(info.st_mode))
            return "inode/fifo";
        if (S_ISSOCK(info.st_mode))
            return "inode/socket";
    }

    const size_t slash = path.rfind('/');
    GlobMatches matches = matchGlobs(slash == std::string::npos ? std::string_view(path)
                                                                : std::string_view(path).substr(slash + 1));
    if (matches.all.size() == 1)
        return matches.all.front();

    // The name is unknown or ambiguous: the content decides.
    std::string data;
    bool readable = false;
    const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd >= 0) {
        readable = true;
        data.resize(kSniffBytes);
        size_t filled = 0;
        while (filled < data.size()) {
            const ssize_t count = read(fd, data.data() + filled, data.size() - filled);
            if (count <= 0)
                break;
            filled += size_t(count);
        }
        data.resize(filled);
        close(fd);
    }
    if (readable) {
        int accuracy = 0;
        const std::string sniffed = data.empty() ? std::string() : sniff(data, &accuracy);
        if (!sniffed.empty() && accuracy > 0) {
            if (std::find(matches.best.begin(), matches.best.end(), sniffed) != matches.best.end())
                return sniffed;
            for (const std::string& type : matches.all) {
                if (inherits(type, sniffed))
                    return type;
            }
            if (matches.all.empty())
                return sniffed;
        }
    }
    if (matches.all.size() > 1) {
        std::sort(matches.best.begin(), matches.best.end());
        return matches.best.front();
    }
    return readable ? typeForData(data) : "application/octet-stream";
}

std::string iconName(std::string_view type)
{
    const std::string name = canonicalName(type).empty() ? std::string(type) : canonicalName(type);
    const auto it = detection().icons.find(name);
    if (it != detection().icons.end())
        return it->second;
    std::string icon = name;
    std::replace(icon.begin(), icon.end(), '/', '-');
    return icon;
}

std::string genericIconName(std::string_view type)
{
    const std::string name = canonicalName(type).empty() ? std::string(type) : canonicalName(type);
    const auto it = detection().genericIcons.find(name);
    if (it != detection().genericIcons.end())
        return it->second;
    return name.substr(0, name.find('/')) + "-x-generic";
}

} // namespace Lingmo::base::mime
