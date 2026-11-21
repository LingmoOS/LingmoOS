// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mime_sniffer_p.h"

#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <map>
#include <set>
#include <span>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <unistd.h>
#include <unordered_map>

#include "base/file.h"
#include "base/mime_database.h"
#include "base/paths.h"
#include "base/strings.h"

namespace Lingmo::Files::internal {

using namespace Lingmo::Base;

// How much of a file content matching reads.
static constexpr size_t SniffLength = 16384;

static const std::string DefaultType = "application/octet-stream";

static std::mutex s_userDataMutex;
static std::string s_userDataDirectory;

void setUserDataDirectory(const std::string& path)
{
    const std::lock_guard lock(s_userDataMutex);
    s_userDataDirectory = path;
}

static bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string userDataDirectory()
{
    const std::lock_guard lock(s_userDataMutex);
    return s_userDataDirectory.empty() ? paths::dataHome() : s_userDataDirectory;
}

std::vector<std::string> mimeDirectories()
{
    std::vector<std::string> data{userDataDirectory()};
    for (const std::string& directory : paths::dataDirectories())
        data.push_back(directory);

    std::vector<std::string> result;
    for (const std::string& directory : data) {
        const std::string mime = directory + "/mime";
        if (isDirectory(mime) && std::find(result.cbegin(), result.cend(), mime) == result.cend())
            result.push_back(mime);
    }

    return result;
}

static int64_t modifiedMs(const std::string& path)
{
    struct stat st;
    if (::stat(path.c_str(), &st) != 0)
        return 0;
    return int64_t(st.st_mtim.tv_sec) * 1000 + st.st_mtim.tv_nsec / 1000000;
}

// What a reload has to notice: the directories, and each cache's replacement.
static std::string databaseVersion(const std::vector<std::string>& directories)
{
    std::string version;
    for (const std::string& directory : directories)
        version += directory + ' ' + strings::number((long long)modifiedMs(directory + "/mime.cache")) + '\n';

    return version;
}

static std::vector<std::string> entries(const std::string& path, bool directories)
{
    std::vector<std::string> result;
    DIR* dir = ::opendir(path.c_str());
    if (!dir)
        return result;

    while (const dirent* entry = ::readdir(dir)) {
        const std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        const std::string full = path + '/' + name;
        struct stat st;
        if (::stat(full.c_str(), &st) != 0)
            continue;
        if (directories ? S_ISDIR(st.st_mode) : S_ISREG(st.st_mode))
            result.push_back(name);
    }
    ::closedir(dir);

    std::sort(result.begin(), result.end());
    return result;
}

// Full Unicode lower case, locale independent.
static std::u16string toLower(const std::u16string& text)
{
    if (std::all_of(text.cbegin(), text.cend(), [](char16_t c) {
            return c < 0x80 && !(c >= 'A' && c <= 'Z');
        }))
        return text;

    std::u16string result(text.size() + 8, u'\0');
    UErrorCode error = U_ZERO_ERROR;
    int32_t length = u_strToLower(reinterpret_cast<UChar*>(result.data()), int32_t(result.size()),
                                  reinterpret_cast<const UChar*>(text.data()), int32_t(text.size()), "", &error);
    if (error == U_BUFFER_OVERFLOW_ERROR) {
        result.assign(size_t(length), u'\0');
        error = U_ZERO_ERROR;
        length = u_strToLower(reinterpret_cast<UChar*>(result.data()), int32_t(result.size()),
                              reinterpret_cast<const UChar*>(text.data()), int32_t(text.size()), "", &error);
    }
    if (U_FAILURE(error))
        return text;

    result.resize(size_t(length));
    return result;
}

static std::u16string latin1(const char* text)
{
    std::u16string result;
    for (const char* c = text; *c; ++c)
        result += char16_t(static_cast<unsigned char>(*c));
    return result;
}

static bool isDigit(char16_t c)
{
    return u_isdigit(c);
}

// A wildcard matching the whole name: '*' any run,
// '?' one character, '[...]' a set with ranges, '!' or '^' negating it.
static bool wildcardMatches(std::u16string_view pattern, std::u16string_view text)
{
    size_t p = 0;
    size_t t = 0;
    size_t starPattern = std::u16string_view::npos;
    size_t starText = 0;

    const auto matchSet = [&](size_t at, char16_t c, size_t* end) {
        size_t i = at + 1;
        bool negate = false;
        if (i < pattern.size() && (pattern[i] == u'!' || pattern[i] == u'^')) {
            negate = true;
            ++i;
        }

        bool matched = false;
        bool first = true;
        while (i < pattern.size() && (first || pattern[i] != u']')) {
            first = false;
            if (i + 2 < pattern.size() && pattern[i + 1] == u'-' && pattern[i + 2] != u']') {
                matched = matched || (c >= pattern[i] && c <= pattern[i + 2]);
                i += 3;
            } else {
                matched = matched || c == pattern[i];
                ++i;
            }
        }

        // An unclosed bracket is a literal.
        if (i >= pattern.size()) {
            *end = at + 1;
            return c == u'[';
        }

        *end = i + 1;
        return matched != negate;
    };

    while (t < text.size()) {
        if (p < pattern.size() && pattern[p] == u'*') {
            starPattern = p++;
            starText = t;
            continue;
        }

        if (p < pattern.size()) {
            size_t next = p + 1;
            bool matched;
            if (pattern[p] == u'?')
                matched = true;
            else if (pattern[p] == u'[')
                matched = matchSet(p, text[t], &next);
            else
                matched = pattern[p] == text[t];

            if (matched) {
                p = next;
                ++t;
                continue;
            }
        }

        if (starPattern == std::u16string_view::npos)
            return false;

        p = starPattern + 1;
        t = ++starText;
    }

    while (p < pattern.size() && pattern[p] == u'*')
        ++p;

    return p == pattern.size();
}

namespace {

// Name matches: the highest weight wins, then the longest pattern.
struct NameMatches
{
    void add(const std::string& type, int weight, size_t patternLength)
    {
        if (std::find(all.cbegin(), all.cend(), type) != all.cend())
            return;

        if (weight < m_weight) {
            all.push_back(type);
            return;
        }

        bool replace = weight > m_weight;
        if (!replace) {
            if (patternLength < m_patternLength)
                return;
            replace = patternLength > m_patternLength;
        }

        if (replace) {
            matching.clear();
            m_patternLength = patternLength;
            m_weight = weight;
        }

        if (std::find(matching.cbegin(), matching.cend(), type) == matching.cend()) {
            matching.push_back(type);
            if (replace)
                all.insert(all.begin(), type);
            else
                all.push_back(type);
        }
    }

    std::vector<std::string> matching;
    std::vector<std::string> all;

private:
    int m_weight = 0;
    size_t m_patternLength = 0;
};

// A glob, classified as a suffix, prefix, literal or complex pattern.
struct Glob
{
    enum Kind {
        Suffix,
        Prefix,
        Literal,
        Vdr,
        Anim,
        Other
    };

    Glob(const char* source, const char* mimeType, int globWeight, bool sensitive)
        : pattern(sensitive ? latin1(source) : toLower(latin1(source)))
        , type(mimeType)
        , weight(globWeight)
        , caseSensitive(sensitive)
        , kind(classify(pattern))
    {
    }

    static Kind classify(std::u16string_view pattern)
    {
        if (pattern.empty())
            return Other;

        const size_t stars = size_t(std::count(pattern.cbegin(), pattern.cend(), u'*'));
        if (pattern.find(u'[') == std::u16string_view::npos && pattern.find(u'?') == std::u16string_view::npos) {
            if (stars == 1) {
                if (pattern.front() == u'*')
                    return Suffix;
                if (pattern.back() == u'*')
                    return Prefix;
            } else if (stars == 0) {
                return Literal;
            }
        }

        if (pattern == u"[0-9][0-9][0-9].vdr")
            return Vdr;
        if (pattern == u"*.anim[1-9j]")
            return Anim;

        return Other;
    }

    // Whether the glob matches; lowerFileName is the name lowered.
    bool matches(const std::u16string& inputFileName, const std::u16string& lowerFileName) const
    {
        const std::u16string& fileName = caseSensitive ? inputFileName : lowerFileName;
        const size_t patternLength = pattern.size();
        const size_t fileNameLength = fileName.size();

        if (!patternLength)
            return false;

        switch (kind) {
        case Suffix:
            if (fileNameLength + 1 < patternLength)
                return false;
            for (size_t i = 1; i < patternLength; ++i) {
                if (pattern[patternLength - i] != fileName[fileNameLength - i])
                    return false;
            }
            return true;
        case Prefix:
            if (fileNameLength + 1 < patternLength)
                return false;
            for (size_t i = 0; i + 1 < patternLength; ++i) {
                if (pattern[i] != fileName[i])
                    return false;
            }
            return true;
        case Literal:
            return pattern == fileName;
        case Vdr:
            return fileNameLength == 7 && isDigit(fileName[0]) && isDigit(fileName[1]) && isDigit(fileName[2])
                && std::u16string_view(fileName).substr(3, 4) == u".vdr";
        case Anim: {
            if (fileNameLength < 6)
                return false;
            const char16_t last = fileName[fileNameLength - 1];
            return ((isDigit(last) && last != u'0') || last == u'j')
                && std::u16string_view(fileName).substr(fileNameLength - 6, 5) == u".anim";
        }
        case Other:
            break;
        }

        return wildcardMatches(pattern, fileName);
    }

    std::u16string pattern;
    std::string type;
    int weight;
    bool caseSensitive;
    Kind kind;
};

// The type files of one MIME directory and which of them carry glob-deleteall, which
// drops the type's globs from the directories after it.
class TypeOverrides
{
public:
    TypeOverrides() = default;

    explicit TypeOverrides(const std::string& directory)
    {
        for (const std::string& medium : entries(directory, true)) {
            for (const std::string& file : entries(directory + '/' + medium, false)) {
                if (!file.ends_with(".xml"))
                    continue;

                const bool deletes = readsGlobDeleteAll(directory + '/' + medium + '/' + file);
                m_types.emplace(medium + '/' + file.substr(0, file.size() - 4), deletes);
                m_deletes = m_deletes || deletes;
            }
        }
    }

    // Whether the type's XML file deletes its globs; the lowercased file first.
    bool deletesGlobs(const std::string& type) const
    {
        if (!m_deletes)
            return false;

        auto found = m_types.find(strings::toAsciiLower(type));
        if (found == m_types.cend())
            found = m_types.find(type);

        return found != m_types.cend() && found->second;
    }

private:
    // Only the mime-type element, then glob-deleteall among its direct
    // children.
    static bool readsGlobDeleteAll(const std::string& path)
    {
        const std::optional<std::string> contents = file::read(path);
        if (!contents || contents->find("glob-deleteall") == std::string::npos)
            return false;

        const std::string& xml = *contents;
        size_t at = 0;
        int depth = 0;
        bool inType = false;

        while ((at = xml.find('<', at)) != std::string::npos) {
            if (xml.compare(at, 4, "<!--") == 0) {
                at = xml.find("-->", at);
                if (at == std::string::npos)
                    return false;
                continue;
            }
            if (xml.compare(at, 2, "<?") == 0 || xml.compare(at, 2, "<!") == 0) {
                at = xml.find('>', at);
                if (at == std::string::npos)
                    return false;
                continue;
            }

            const size_t end = xml.find('>', at);
            if (end == std::string::npos)
                return false;

            const std::string_view tag(xml.data() + at + 1, end - at - 1);
            at = end + 1;

            if (tag.starts_with('/')) {
                --depth;
                if (inType && depth == 0)
                    return false;
                continue;
            }

            const bool selfClosing = tag.ends_with('/');
            const size_t nameEnd = tag.find_first_of(" \t\r\n/");
            const std::string_view name = tag.substr(0, nameEnd);

            if (!inType) {
                // The first element must be a mime-type with a type.
                if (name != "mime-type" || tag.find("type=") == std::string_view::npos)
                    return false;
                inType = true;
                depth = selfClosing ? 0 : 1;
                if (selfClosing)
                    return false;
                continue;
            }

            if (depth == 1 && name == "glob-deleteall")
                return true;
            if (!selfClosing)
                ++depth;
        }

        return false;
    }

    std::unordered_map<std::string, bool> m_types;
    bool m_deletes = false;
};

} // namespace

// Whether a directory before this one deletes the type's globs.
static bool globsExcluded(std::span<const TypeOverrides> earlier, const std::string& type)
{
    for (const TypeOverrides& overrides : earlier) {
        if (overrides.deletesGlobs(type))
            return true;
    }

    return false;
}

namespace {

// A mime.cache file, read trusting the offsets it holds.
class CacheFile
{
public:
    explicit CacheFile(const std::string& path)
    {
        const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd < 0)
            return;

        struct stat st;
        if (::fstat(fd, &st) == 0 && st.st_size >= 4) {
            void* data = ::mmap(nullptr, size_t(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
            if (data != MAP_FAILED) {
                m_data = static_cast<const uint8_t*>(data);
                m_size = size_t(st.st_size);
            }
        }
        ::close(fd);

        if (!m_data)
            return;

        const int major = uint16(0);
        const int minor = uint16(2);
        m_valid = major == 1 && minor >= 1 && minor <= 2;

        if (m_valid) {
            m_literals = globList(uint32(12));
            m_globs = globList(uint32(20));
        }
    }

    ~CacheFile()
    {
        if (m_data)
            ::munmap(const_cast<uint8_t*>(m_data), m_size);
    }

    CacheFile(const CacheFile&) = delete;
    CacheFile& operator=(const CacheFile&) = delete;

    bool isValid() const { return m_valid; }

    // The types whose globs match the name, skipping types whose globs an
    // earlier directory deletes.
    void addFileNameMatches(const std::u16string& fileName, const std::u16string& lowerFileName,
                            std::span<const TypeOverrides> earlier, NameMatches* result) const
    {
        bool matched = matchGlobs(m_literals, fileName, lowerFileName, earlier, result);

        if (!matched) {
            const int treeOffset = int(uint32(16));
            const int numRoots = int(uint32(treeOffset));
            const int firstRootOffset = int(uint32(treeOffset + 4));

            matched = matchSuffixTree(result, earlier, numRoots, firstRootOffset, lowerFileName,
                                      ptrdiff_t(lowerFileName.size()) - 1, false)
                || matchSuffixTree(result, earlier, numRoots, firstRootOffset, fileName,
                                   ptrdiff_t(fileName.size()) - 1, true);
        }

        if (!matched)
            matchGlobs(m_globs, fileName, lowerFileName, earlier, result);
    }

    // The first magic rule that matches, if it is
    // more accurate than what an earlier cache found.
    void findByMagic(std::string_view data, int* accuracy, const char** type) const
    {
        const int magicListOffset = int(uint32(24));
        const int numMatches = int(uint32(magicListOffset));
        const int firstMatchOffset = int(uint32(magicListOffset + 8));

        for (int i = 0; i < numMatches; ++i) {
            const int offset = firstMatchOffset + i * 16;

            if (matchRule(int(uint32(offset + 8)), int(uint32(offset + 12)), data)) {
                const int matchAccuracy = int(uint32(offset));

                if (matchAccuracy > *accuracy) {
                    *accuracy = matchAccuracy;
                    *type = string(int(uint32(offset + 4)));
                    return;
                }
            }
        }
    }

private:
    uint16_t uint16(int offset) const
    {
        return uint16_t(m_data[offset] << 8 | m_data[offset + 1]);
    }

    uint32_t uint32(int offset) const
    {
        return uint32_t(m_data[offset]) << 24 | uint32_t(m_data[offset + 1]) << 16 | uint32_t(m_data[offset + 2]) << 8
            | uint32_t(m_data[offset + 3]);
    }

    const char* string(int offset) const { return reinterpret_cast<const char*>(m_data + offset); }

    std::vector<Glob> globList(int offset) const
    {
        std::vector<Glob> globs;
        const int count = int(uint32(offset));
        globs.reserve(size_t(count));

        for (int i = 0; i < count; ++i) {
            const int entry = offset + 4 + 12 * i;
            const int flagsAndWeight = int(uint32(entry + 8));
            globs.emplace_back(string(int(uint32(entry))), string(int(uint32(entry + 4))), flagsAndWeight & 0xff,
                               flagsAndWeight & 0x100);
        }

        return globs;
    }

    static bool matchGlobs(const std::vector<Glob>& globs, const std::u16string& fileName,
                           const std::u16string& lowerFileName, std::span<const TypeOverrides> earlier,
                           NameMatches* result)
    {
        bool matched = false;
        for (const Glob& glob : globs) {
            if (globsExcluded(earlier, glob.type))
                continue;

            if (glob.matches(fileName, lowerFileName)) {
                result->add(glob.type, glob.weight, glob.pattern.size());
                matched = true;
            }
        }

        return matched;
    }

    // Matches the file name from its last character back, along a tree of
    // reversed extensions.
    bool matchSuffixTree(NameMatches* result, std::span<const TypeOverrides> earlier, int numEntries,
                         int firstOffset, const std::u16string& fileName, ptrdiff_t charPos,
                         bool caseSensitiveCheck) const
    {
        if (charPos < 0)
            return false;

        const char16_t fileChar = fileName[size_t(charPos)];
        int min = 0;
        int max = numEntries - 1;

        while (min <= max) {
            const int mid = (min + max) / 2;
            const int offset = firstOffset + 12 * mid;
            const char16_t ch = char16_t(uint32(offset));

            if (ch < fileChar) {
                min = mid + 1;
            } else if (ch > fileChar) {
                max = mid - 1;
            } else {
                --charPos;
                const int numChildren = int(uint32(offset + 4));
                const int childrenOffset = int(uint32(offset + 8));
                bool success = false;

                if (charPos > 0)
                    success = matchSuffixTree(result, earlier, numChildren, childrenOffset, fileName, charPos,
                                              caseSensitiveCheck);

                if (!success) {
                    for (int i = 0; i < numChildren; ++i) {
                        const int childOffset = childrenOffset + 12 * i;
                        if (uint32(childOffset) != 0)
                            break;

                        const std::string type = string(int(uint32(childOffset + 4)));
                        if (globsExcluded(earlier, type))
                            continue;

                        const int flagsAndWeight = int(uint32(childOffset + 8));
                        const bool caseSensitive = flagsAndWeight & 0x100;

                        if (caseSensitiveCheck || !caseSensitive) {
                            // The pattern is '*' and the characters matched so far.
                            result->add(type, flagsAndWeight & 0xff, size_t(ptrdiff_t(fileName.size()) - charPos));
                            success = true;
                        }
                    }
                }

                return success;
            }
        }

        return false;
    }

    bool matchRule(int numMatchlets, int firstOffset, std::string_view data) const
    {
        for (int matchlet = 0; matchlet < numMatchlets; ++matchlet) {
            const int offset = firstOffset + matchlet * 32;
            const int rangeStart = int(uint32(offset));
            const int rangeLength = int(uint32(offset + 4));
            const int valueLength = int(uint32(offset + 12));
            const int maskOffset = int(uint32(offset + 20));

            if (!matchSubstring(data.data(), ptrdiff_t(data.size()), rangeStart, rangeLength, valueLength,
                                string(int(uint32(offset + 16))), maskOffset ? string(maskOffset) : nullptr))
                continue;

            const int numChildren = int(uint32(offset + 24));
            if (numChildren == 0 || matchRule(numChildren, int(uint32(offset + 28)), data))
                return true;
        }

        return false;
    }

    // The value at any position from rangeStart on, fewer than rangeLength
    // positions later; every position in that range is tried.
    static bool matchSubstring(const char* data, ptrdiff_t dataSize, int rangeStart, int rangeLength,
                               ptrdiff_t valueLength, const char* value, const char* mask)
    {
        if (!mask) {
            if (rangeLength <= 0 || rangeStart + valueLength > dataSize)
                return false;

            const ptrdiff_t end = std::min(dataSize, ptrdiff_t(rangeStart) + rangeLength - 1 + valueLength);
            return ::memmem(data + rangeStart, size_t(end - rangeStart), value, size_t(valueLength)) != nullptr;
        }

        const ptrdiff_t dataNeeded = std::min(ptrdiff_t(rangeLength) + valueLength - 1, dataSize - rangeStart);
        const ptrdiff_t maxStartPos = dataNeeded - valueLength + 1;

        for (ptrdiff_t i = 0; i < maxStartPos; ++i) {
            const char* d = data + rangeStart + i;
            ptrdiff_t index = 0;

            while (index < valueLength && (d[index] & mask[index]) == (value[index] & mask[index]))
                ++index;

            if (index == valueLength)
                return true;
        }

        return false;
    }

    const uint8_t* m_data = nullptr;
    size_t m_size = 0;
    bool m_valid = false;
    std::vector<Glob> m_literals;
    std::vector<Glob> m_globs;
};

} // namespace

// A UTF-16 byte order mark, or no control character but tab, line feed and
// carriage return in the first 128 bytes.
static bool isTextFile(std::string_view data)
{
    if (data.starts_with("\xFE\xFF") || data.starts_with("\xFF\xFE"))
        return true;

    const size_t end = std::min<size_t>(128, data.size());
    for (size_t i = 0; i < end; ++i) {
        const char c = data[i];
        if (static_cast<unsigned char>(c) < 32 && c != 9 && c != 10 && c != 13)
            return false;
    }

    return true;
}

static std::vector<std::pair<std::string, std::string>> pairs(const std::string& path)
{
    std::vector<std::pair<std::string, std::string>> result;
    const std::optional<std::string> contents = file::read(path);
    if (!contents)
        return result;

    for (const std::string_view line : strings::splitViews(*contents, '\n')) {
        const size_t space = line.find(' ');
        if (line.empty() || line.front() == '#' || space == std::string_view::npos)
            continue;
        result.emplace_back(std::string(line.substr(0, space)), std::string(line.substr(space + 1)));
    }

    return result;
}

struct MimeSniffer::Database
{
    // The type the magic rules give the data, and its accuracy.
    std::pair<std::string, int> findByData(std::string_view data) const
    {
        if (data.empty())
            return {"application/x-zerosize", 100};

        int accuracy = 0;
        const char* type = nullptr;
        for (const auto& cache : caches)
            cache->findByMagic(data, &accuracy, &type);

        if (type)
            return {type, accuracy};

        if (isTextFile(data))
            return {"text/plain", 5};

        return {DefaultType, 0};
    }

    // The canonical name, aliases resolved; empty
    // for a type no directory knows.
    std::string canonicalName(const std::string& name) const
    {
        const auto alias = aliases.find(name);
        const std::string& type = alias != aliases.cend() ? alias->second : name;
        return types.contains(type) ? type : std::string();
    }

    std::vector<std::string> parentsOf(const std::string& name) const
    {
        const auto found = parents.find(name);
        if (found != parents.cend() && !found->second.empty())
            return found->second;

        // Implicit parents: text/plain for text, application/octet-stream for most others.
        const std::string group = name.substr(0, name.find('/'));
        if (group == "text" && name != "text/plain")
            return {"text/plain"};
        if (group != "inode" && group != "all" && group != "fonts" && group != "print" && group != "uri"
            && name != DefaultType)
            return {DefaultType};
        return {};
    }

    // Whether type is ancestor or descends from it, aliases resolved.
    bool inherits(const std::string& type, const std::string& ancestor) const
    {
        const std::string known = canonicalName(type);
        const std::string start = known.empty() ? type : known;
        const std::string wanted = canonicalName(ancestor).empty() ? ancestor : canonicalName(ancestor);

        std::vector<std::string> pending{start};
        std::set<std::string> seen;
        while (!pending.empty()) {
            const std::string current = pending.back();
            pending.pop_back();
            if (current == wanted)
                return true;
            if (!seen.insert(current).second)
                continue;
            for (const std::string& parent : parentsOf(current)) {
                const std::string resolved = canonicalName(parent).empty() ? parent : canonicalName(parent);
                pending.push_back(resolved);
            }
        }

        return false;
    }

    std::string version;
    // Unset when a directory has XML packages but no valid mime.cache, or the
    // shared freedesktop.org database is missing.
    bool usable = true;
    std::vector<std::unique_ptr<CacheFile>> caches;
    // Per cache; the last one's are never consulted.
    std::vector<TypeOverrides> overrides;
    std::set<std::string, std::less<>> types;
    std::unordered_map<std::string, std::string> aliases;
    std::unordered_map<std::string, std::vector<std::string>> parents;
};

MimeSniffer* MimeSniffer::self()
{
    static MimeSniffer* sniffer = new MimeSniffer;
    return sniffer;
}

// The caches are checked for changes at most every five seconds.
std::shared_ptr<const MimeSniffer::Database> MimeSniffer::database()
{
    const std::lock_guard lock(m_mutex);

    if (m_database && m_lastCheck.isValid() && m_lastCheck.elapsedMs() < 5000)
        return m_database;

    m_lastCheck.start();

    const std::vector<std::string> directories = mimeDirectories();
    const std::string version = databaseVersion(directories);
    if (m_database && m_database->version == version)
        return m_database;

    auto database = std::make_shared<Database>();
    database->version = version;

    bool freedesktop = false;
    std::vector<std::string> cacheDirectories;
    for (const std::string& directory : directories) {
        const std::string packages = directory + "/packages";
        struct stat st;
        freedesktop = freedesktop || ::stat((packages + "/freedesktop.org.xml").c_str(), &st) == 0;

        auto cache = std::make_unique<CacheFile>(directory + "/mime.cache");
        if (cache->isValid()) {
            database->caches.push_back(std::move(cache));
            cacheDirectories.push_back(directory);
        } else {
            const std::vector<std::string> files = entries(packages, false);
            if (std::any_of(files.cbegin(), files.cend(), [](const std::string& name) {
                    return name.ends_with(".xml");
                }))
                database->usable = false;
        }

        // The user's names first, in the directories' order.
        if (const std::optional<std::string> types = file::read(directory + "/types")) {
            for (const std::string_view line : strings::splitViews(*types, '\n')) {
                if (!line.empty())
                    database->types.emplace(line);
            }
        }
        for (const auto& [alias, type] : pairs(directory + "/aliases"))
            database->aliases.emplace(alias, type);
        for (const auto& [type, parent] : pairs(directory + "/subclasses")) {
            auto& list = database->parents[type];
            if (std::find(list.cbegin(), list.cend(), parent) == list.cend())
                list.push_back(parent);
        }
    }

    for (size_t i = 0; i < cacheDirectories.size(); ++i) {
        if (i + 1 < cacheDirectories.size())
            database->overrides.emplace_back(cacheDirectories[i]);
        else
            database->overrides.emplace_back();
    }

    // Without the shared freedesktop.org database these caches miss its rules.
    database->usable = database->usable && freedesktop;

    m_database = database;
    return m_database;
}

// The type of a single glob match, else the content's magic type, refined to a
// glob-matched type that inherits it, else the first of several glob matches.
std::string MimeSniffer::mimeTypeForFile(const std::string& path)
{
    const auto current = database();

    if (!current->usable)
        return mime::typeForFile(path);

    NameMatches byName;
    const std::string fileName = path.substr(path.rfind('/') + 1);
    if (!fileName.empty()) {
        const std::u16string name = strings::utf8ToUtf16(fileName);
        const std::u16string lowerName = toLower(name);
        for (size_t i = 0; i < current->caches.size(); ++i) {
            current->caches[i]->addFileNameMatches(name, lowerName, std::span(current->overrides).first(i),
                                                   &byName);
        }
    }

    if (byName.all.size() == 1) {
        const std::string type = current->canonicalName(byName.matching.front());
        if (!type.empty())
            return type;

        byName = {};
    }

    // Unreadable contents leave the name alone to decide.
    const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd >= 0) {
        std::string data(SniffLength, '\0');
        size_t filled = 0;
        while (filled < data.size()) {
            const ssize_t count = ::read(fd, data.data() + filled, data.size() - filled);
            if (count < 0 && errno == EINTR)
                continue;
            if (count <= 0)
                break;
            filled += size_t(count);
        }
        data.resize(filled);
        ::close(fd);

        const auto [sniffed, accuracy] = current->findByData(data);
        if (accuracy > 0) {
            std::string type;

            if (std::find(byName.matching.cbegin(), byName.matching.cend(), sniffed) != byName.matching.cend()
                || byName.all.empty()) {
                type = sniffed;
            } else {
                for (const std::string& candidate : byName.all) {
                    if (current->inherits(candidate, sniffed)) {
                        type = candidate;
                        break;
                    }
                }
            }

            if (!type.empty()) {
                const std::string result = current->canonicalName(type);
                // A magic match is returned unchecked; a type no directory knows is the toolkit's call.
                return !result.empty() ? result : mime::typeForFile(path);
            }
        }
    }

    if (byName.all.size() > 1) {
        std::sort(byName.matching.begin(), byName.matching.end());
        const std::string type = current->canonicalName(byName.matching.front());
        if (!type.empty())
            return type;
    }

    return DefaultType;
}

bool MimeSniffer::inherits(const std::string& type, const std::string& ancestor)
{
    return database()->inherits(type, ancestor);
}

std::string MimeSniffer::canonicalName(const std::string& name)
{
    return database()->canonicalName(name);
}

} // namespace Lingmo::Files::internal
