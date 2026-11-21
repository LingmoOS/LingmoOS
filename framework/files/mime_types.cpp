// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/mime_types.h"

#include <cstdlib>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

#include "base/file.h"
#include "base/locale.h"
#include "base/mime_database.h"
#include "mime_sniffer_p.h"

namespace Lingmo::Files::mime {

using namespace Lingmo::Base;
using internal::MimeSniffer;

std::string typeForFile(const std::string& path)
{
    struct stat st;
    if (::stat(path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode))
            return "inode/directory";
        if (S_ISCHR(st.st_mode))
            return "inode/chardevice";
        if (S_ISBLK(st.st_mode))
            return "inode/blockdevice";
        if (S_ISFIFO(st.st_mode))
            return "inode/fifo";
        if (S_ISSOCK(st.st_mode))
            return "inode/socket";
    }

    return MimeSniffer::self()->mimeTypeForFile(path);
}

std::string typeForRegularFile(const std::string& path)
{
    return MimeSniffer::self()->mimeTypeForFile(path);
}

std::string canonicalName(const std::string& name)
{
    return MimeSniffer::self()->canonicalName(name);
}

bool inherits(const std::string& type, const std::string& ancestor)
{
    return MimeSniffer::self()->inherits(type, ancestor);
}

static std::string decodeEntities(std::string_view text)
{
    std::string result;
    result.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '&') {
            result += text[i];
            continue;
        }

        const size_t end = text.find(';', i);
        if (end == std::string_view::npos) {
            result += text[i];
            continue;
        }

        const std::string_view entity = text.substr(i + 1, end - i - 1);
        if (entity == "amp")
            result += '&';
        else if (entity == "lt")
            result += '<';
        else if (entity == "gt")
            result += '>';
        else if (entity == "quot")
            result += '"';
        else if (entity == "apos")
            result += '\'';
        else if (entity.starts_with('#')) {
            const bool hex = entity.size() > 1 && (entity[1] == 'x' || entity[1] == 'X');
            const std::string digits(entity.substr(hex ? 2 : 1));
            char32_t value = char32_t(std::strtoul(digits.c_str(), nullptr, hex ? 16 : 10));
            if (value) {
                if (value < 0x80) {
                    result += char(value);
                } else if (value < 0x800) {
                    result += char(0xC0 | (value >> 6));
                    result += char(0x80 | (value & 0x3F));
                } else if (value < 0x10000) {
                    result += char(0xE0 | (value >> 12));
                    result += char(0x80 | ((value >> 6) & 0x3F));
                    result += char(0x80 | (value & 0x3F));
                } else {
                    result += char(0xF0 | (value >> 18));
                    result += char(0x80 | ((value >> 12) & 0x3F));
                    result += char(0x80 | ((value >> 6) & 0x3F));
                    result += char(0x80 | (value & 0x3F));
                }
            }
        } else {
            result += text.substr(i, end - i + 1);
        }
        i = end;
    }

    return result;
}

// The comment elements of a type file, by xml:lang; "default" for the one
// without.
static std::map<std::string, std::string> comments(const std::string& type)
{
    std::map<std::string, std::string> result;

    for (const std::string& directory : internal::mimeDirectories()) {
        const std::optional<std::string> contents = file::read(directory + '/' + type + ".xml");
        if (!contents)
            continue;

        const std::string& xml = *contents;
        size_t at = 0;
        while ((at = xml.find("<comment", at)) != std::string::npos) {
            const size_t tagEnd = xml.find('>', at);
            if (tagEnd == std::string::npos)
                break;

            const std::string_view tag(xml.data() + at, tagEnd - at);
            at = tagEnd + 1;
            if (tag.size() > 8 && tag[8] != ' ' && tag[8] != '\t' && tag[8] != '\n')
                continue;
            if (tag.ends_with('/'))
                continue;

            const size_t close = xml.find("</comment>", at);
            if (close == std::string::npos)
                break;

            std::string language = "default";
            const size_t lang = tag.find("xml:lang=");
            if (lang != std::string_view::npos && lang + 10 <= tag.size()) {
                const char quote = tag[lang + 9];
                const size_t end = tag.find(quote, lang + 10);
                if (end != std::string_view::npos)
                    language = std::string(tag.substr(lang + 10, end - lang - 10));
            }

            result.emplace(language, decodeEntities(std::string_view(xml).substr(at, close - at)));
            at = close + 10;
        }

        // The first directory that has the type describes it.
        return result;
    }

    return result;
}

std::string description(const std::string& type)
{
    // By the Locale::system() of the time, which a language change replaces.
    static std::mutex mutex;
    static std::unordered_map<const Locale*, std::unordered_map<std::string, std::string>> caches;

    const Locale* locale = &Locale::system();
    {
        const std::lock_guard lock(mutex);
        const auto& cache = caches[locale];
        if (const auto found = cache.find(type); found != cache.cend())
            return found->second;
    }

    const std::map<std::string, std::string> all = comments(type);

    // The session's messages locale, then its language alone, then the
    // untranslated description; "C" reads as en_US.
    std::string name = locale->name();
    if (name == "C")
        name = "en_US";

    std::vector<std::string> languages{name};
    if (const size_t cut = name.find('_'); cut != std::string::npos)
        languages.push_back(name.substr(0, cut));
    languages.push_back("default");

    std::string text = type;
    for (const std::string& language : languages) {
        if (const auto found = all.find(language); found != all.cend() && !found->second.empty()) {
            text = found->second;
            break;
        }
    }

    const std::lock_guard lock(mutex);
    caches[locale].emplace(type, text);
    return text;
}

std::string iconName(const std::string& type)
{
    return base::mime::iconName(type);
}

} // namespace Lingmo::Files::mime
