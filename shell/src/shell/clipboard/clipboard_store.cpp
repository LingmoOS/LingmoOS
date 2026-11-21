// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/clipboard/clipboard_store.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <set>

#include <sys/stat.h>

#include "base/file.h"
#include "base/hash.h"
#include "base/json.h"
#include "base/paths.h"
#include "base/strings.h"
#include "base/url.h"
#include "core/preferences.h"
#include "shell/common/file_thread.h"
#include <string_view>

namespace Lingmo::Shell {

// A SHA-256 in hex, as keys and body file names are.
static bool isBodyName(const std::string &name)
{
    return name.size() == 64
            && std::all_of(name.begin(), name.end(), [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}

namespace fs = std::filesystem;

static constexpr const char *kIndexName = "index.json";
static constexpr const char *kUriList = "text/uri-list";
// Password managers mark their copies with it.
static constexpr const char *kPasswordHint = "x-kde-passwordManagerHint";

static const std::vector<std::string> &textTypes()
{
    static const std::vector<std::string> types = {"text/plain;charset=utf-8", "text/plain", "UTF8_STRING",
                                                   "STRING"};
    return types;
}

static bool contains(const std::vector<std::string> &list, const std::string &value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

// The first `units` UTF-16 units of the text, without splitting a character.
static std::string leftUtf16(const std::string &text, size_t units)
{
    if (base::strings::utf16Length(text) <= units)
        return text;
    const std::u16string utf16 = base::strings::utf8ToUtf16(text);
    size_t size = units;
    if (size > 0 && utf16[size - 1] >= 0xd800 && utf16[size - 1] <= 0xdbff)
        --size;
    return base::strings::utf16ToUtf8(std::u16string_view(utf16).substr(0, size));
}

static int64_t nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

static bool ensureDirectory(const std::string &directory)
{
    std::error_code error;
    fs::create_directories(directory, error);
    return chmod(directory.c_str(), 0700) == 0;
}

static base::json::Value entryToJson(const ClipboardEntry &entry)
{
    base::json::Value value = base::json::Value::object();
    value.insert("key", entry.key);
    value.insert("kind", int(entry.kind));
    if (entry.copied)
        value.insert("copied", *entry.copied);
    value.insert("text", entry.text);
    base::json::Value::Array files;
    for (const std::string &file : entry.files)
        files.push_back(file);
    value.insert("files", std::move(files));
    value.insert("imageType", entry.imageType);
    return value;
}

static ClipboardEntry entryFromJson(const base::json::Value &value)
{
    ClipboardEntry entry;
    entry.key = value["key"].toString();
    const int64_t kind = value["kind"].toInt(-1);
    if (kind < int64_t(ClipboardEntry::Kind::Text) || kind > int64_t(ClipboardEntry::Kind::Files))
        return {};
    entry.kind = ClipboardEntry::Kind(kind);
    if (value["copied"].isNumber())
        entry.copied = value["copied"].toInt();
    entry.text = value["text"].toString();
    if (value["files"].isArray()) {
        for (const base::json::Value &file : value["files"].toArray())
            entry.files.push_back(file.toString());
    }
    entry.imageType = value["imageType"].toString();
    return entry;
}

ClipboardStore::ClipboardStore(std::string directory, size_t limit)
    : m_directory(std::move(directory))
    , m_limit(limit)
{
    m_load = runOnFileThread(
            [directory = m_directory, limit = m_limit] {
                std::vector<ClipboardEntry> entries;
                if (const auto data = base::file::read(directory + '/' + kIndexName)) {
                    const std::optional<base::json::Value> index = base::json::Value::parse(*data);
                    if (index && index->isArray()) {
                        for (const base::json::Value &record : index->toArray())
                            entries.push_back(entryFromJson(record));
                    }
                }

                // An entry whose body is gone cannot be pasted.
                std::error_code error;
                std::erase_if(entries, [&](const ClipboardEntry &entry) {
                    return entry.key.empty() || entry.key.find('/') != std::string::npos
                            || !fs::exists(directory + '/' + entry.key, error);
                });
                if (entries.size() > limit)
                    entries.resize(limit);

                std::set<std::string> named{kIndexName};
                for (const ClipboardEntry &entry : entries)
                    named.insert(entry.key);
                for (const auto &item : fs::directory_iterator(directory, error)) {
                    std::error_code typeError;
                    // Only bodies this store could have written.
                    const std::string name = item.path().filename().string();
                    if (item.is_regular_file(typeError) && !named.count(name) && isBodyName(name))
                        fs::remove(item.path(), typeError);
                }
                return entries;
            },
            [this](std::vector<ClipboardEntry> entries) {
                // Copies made while the index was read are newer.
                const bool copiedMeanwhile = !m_entries.empty();
                for (ClipboardEntry &entry : entries) {
                    if (indexOf(entry.key) < 0)
                        m_entries.push_back(std::move(entry));
                }
                if (m_entries.size() > m_limit)
                    m_entries.resize(m_limit);
                m_loaded = true;
                if (copiedMeanwhile)
                    save();
                changed();
            });
}

ClipboardStore::~ClipboardStore()
{
    waitForFileThread();
}

std::string ClipboardStore::defaultDirectory()
{
    // Not ~/.config/lingmo/clipboard-history, an older history in another
    // format, which is left alone: its files are neither read nor removed.
    return base::paths::dataHome() + "/lingmo/clipboard-history";
}

std::vector<std::string> ClipboardStore::formatsToKeep(const std::vector<std::string> &offered)
{
    if (contains(offered, kPasswordHint))
        return {};

    std::vector<std::string> formats;
    if (contains(offered, kUriList))
        formats.push_back(kUriList);
    for (const std::string &type : textTypes()) {
        if (contains(offered, type)) {
            formats.push_back(type);
            break;
        }
    }

    // An image only on its own: office suites offer a picture of every text
    // they copy, which would fill the history with megabytes.
    if (formats.empty()) {
        if (contains(offered, "image/png")) {
            formats.push_back("image/png");
        } else {
            for (const std::string &type : offered) {
                if (type.starts_with("image/")) {
                    formats.push_back(type);
                    break;
                }
            }
        }
    }
    return formats;
}

bool ClipboardStore::describe(const ClipboardPayload &payload, ClipboardEntry *entry)
{
    std::string text;
    std::vector<std::string> files;
    std::string imageType;

    for (const auto &[type, data] : payload) {
        if (type == kUriList) {
            for (const std::string &line : base::strings::split(data, '\n')) {
                const std::string trimmed(base::strings::trimmed(line));
                if (trimmed.empty() || trimmed.starts_with('#'))
                    continue;
                if (const auto path = base::url::toLocalFile(trimmed))
                    files.push_back(*path);
            }
        } else if (contains(textTypes(), type)) {
            if (text.empty())
                text = type == "STRING" ? base::strings::latin1ToUtf8(data) : base::strings::sanitizedUtf8(data);
        } else if (type.starts_with("image/") && !data.empty()) {
            imageType = type;
        }
    }

    if (!files.empty()) {
        entry->kind = ClipboardEntry::Kind::Files;
        entry->files = files;
        entry->text = leftUtf16(base::strings::join(files, "\n"), kIndexedTextLength);
    } else if (!base::strings::trimmed(text).empty()) {
        entry->kind = ClipboardEntry::Kind::Text;
        entry->text = leftUtf16(text, kIndexedTextLength);
    } else if (!imageType.empty()) {
        entry->kind = ClipboardEntry::Kind::Image;
        entry->imageType = imageType;
    } else {
        return false;
    }

    entry->key = keyFor(payload);
    return true;
}

std::string ClipboardStore::keyFor(const ClipboardPayload &payload)
{
    base::CryptographicHash hash(base::CryptographicHash::Algorithm::Sha256);
    for (const auto &[type, data] : payload) {
        hash.addData(type);
        hash.addData(std::string_view("\0", 1));
        // The size in the machine's byte order.
        const uint64_t size = data.size();
        hash.addData(&size, sizeof(size));
        hash.addData(data);
    }
    return base::strings::toHex(hash.result());
}

int ClipboardStore::indexOf(const std::string &key) const
{
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].key == key)
            return int(i);
    }
    return -1;
}

std::string ClipboardStore::bodyPath(const std::string &key) const
{
    return m_directory + '/' + key;
}

bool ClipboardStore::add(const ClipboardPayload &payload, std::optional<int64_t> copied)
{
    int64_t size = 0;
    for (const auto &format : payload)
        size += int64_t(format.second.size());
    if (size > kMaxPayloadSize)
        return false;

    ClipboardEntry entry;
    if (!describe(payload, &entry))
        return false;
    entry.copied = copied ? copied : std::optional<int64_t>(nowMs());

    if (indexOf(entry.key) >= 0) {
        moveToTop(entry.key);
        return true;
    }
    if (!m_writing.insert(entry.key).second)
        return true;

    std::string body;
    for (const auto &[type, data] : payload) {
        body += type + '\n' + std::to_string(data.size()) + '\n';
        body += data;
    }

    std::erase_if(m_jobs, [](const base::Job &job) { return !job.isPending(); });
    m_jobs.push_back(runOnFileThread(
            [directory = m_directory, path = bodyPath(entry.key), data = std::move(body)] {
                if (!ensureDirectory(directory) || !base::file::writeAtomically(path, data))
                    return false;
                // Copies hold whatever the user copied: only the user may read them.
                return chmod(path.c_str(), 0600) == 0;
            },
            [this, entry](bool written) {
                m_writing.erase(entry.key);
                if (!written || indexOf(entry.key) >= 0)
                    return;
                m_entries.insert(m_entries.begin(), entry);
                while (m_entries.size() > m_limit) {
                    fileThread().post([path = bodyPath(m_entries.back().key)] {
                        std::error_code error;
                        fs::remove(path, error);
                    });
                    m_entries.pop_back();
                }
                save();
                changed();
            }));
    return true;
}

void ClipboardStore::moveToTop(const std::string &key)
{
    const int index = indexOf(key);
    if (index <= 0)
        return;
    ClipboardEntry entry = std::move(m_entries[size_t(index)]);
    m_entries.erase(m_entries.begin() + index);
    m_entries.insert(m_entries.begin(), std::move(entry));
    save();
    changed();
}

void ClipboardStore::remove(const std::string &key)
{
    const int index = indexOf(key);
    if (index < 0)
        return;
    m_entries.erase(m_entries.begin() + index);
    fileThread().post([path = bodyPath(key)] {
        std::error_code error;
        fs::remove(path, error);
    });
    save();
    changed();
}

void ClipboardStore::clear()
{
    if (m_entries.empty())
        return;
    std::vector<std::string> paths;
    for (const ClipboardEntry &entry : m_entries)
        paths.push_back(bodyPath(entry.key));
    m_entries.clear();
    fileThread().post([paths] {
        std::error_code error;
        for (const std::string &path : paths)
            fs::remove(path, error);
    });
    save();
    changed();
}

void ClipboardStore::save()
{
    // Before the index is read, a write would lose it.
    if (!m_loaded)
        return;
    base::json::Value::Array index;
    for (const ClipboardEntry &entry : m_entries)
        index.push_back(entryToJson(entry));
    fileThread().post([directory = m_directory, data = base::json::Value(std::move(index)).serialize()] {
        if (!ensureDirectory(directory))
            return;
        const std::string path = directory + '/' + kIndexName;
        if (base::file::writeAtomically(path, data))
            chmod(path.c_str(), 0600);
    });
}

ClipboardPayload ClipboardStore::readBody(const std::string &path)
{
    const auto data = base::file::read(path);
    if (!data)
        return {};
    ClipboardPayload payload;
    std::string_view rest(*data);
    while (!rest.empty()) {
        const size_t typeEnd = rest.find('\n');
        const size_t sizeEnd = typeEnd == std::string_view::npos ? typeEnd : rest.find('\n', typeEnd + 1);
        if (sizeEnd == std::string_view::npos)
            return {};
        const std::string_view sizeText = rest.substr(typeEnd + 1, sizeEnd - typeEnd - 1);
        uint64_t size = 0;
        const auto [end, error] = std::from_chars(sizeText.data(), sizeText.data() + sizeText.size(), size);
        if (error != std::errc() || end != sizeText.data() + sizeText.size() || sizeText.empty()
            || size > rest.size() - sizeEnd - 1)
            return {};
        payload.emplace_back(std::string(rest.substr(0, typeEnd)), std::string(rest.substr(sizeEnd + 1, size)));
        rest.remove_prefix(sizeEnd + 1 + size);
    }
    return payload;
}

void ClipboardStore::payload(const std::string &key, std::function<void(ClipboardPayload)> done)
{
    std::erase_if(m_jobs, [](const base::Job &job) { return !job.isPending(); });
    m_jobs.push_back(runOnFileThread([path = bodyPath(key)] { return readBody(path); }, std::move(done)));
}

} // namespace Lingmo::Shell
