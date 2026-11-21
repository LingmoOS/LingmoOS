// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"

namespace Lingmo::Shell {

// What one copy held: each kept type with its bytes, in the order kept.
using ClipboardPayload = std::vector<std::pair<std::string, std::string>>;

struct ClipboardEntry {
    enum class Kind {
        Text = 0,
        Image = 1,
        Files = 2,
    };

    // The SHA-256 of the payload, in hex; also the name of its body file.
    std::string key;
    Kind kind = Kind::Text;
    // Milliseconds since the epoch.
    std::optional<int64_t> copied;
    // The start of the text, for the list and the filter.
    std::string text;
    // Local paths of copied files.
    std::vector<std::string> files;
    // The image type, for an image.
    std::string imageType;
};

// The clipboard history on disk in ~/.local/share/lingmo/clipboard-history:
// index.json, the entries newest first, and one body file per entry holding
// its payload (each type's name, a newline, its size in decimal, a newline,
// then its bytes). Only the user may read
// them. Every read and write runs on the file thread; the entries here are
// what the index says once a change is through.
//
// A copy whose payload is already in the history moves that entry to the
// top instead of adding one. Bodies are written once and removed with their
// entry.
class ClipboardStore
{
public:
    // A copy larger than this is not kept.
    static constexpr int64_t kMaxPayloadSize = 16 * 1024 * 1024;
    // The part of a text the index keeps, in UTF-16 units.
    static constexpr size_t kIndexedTextLength = 8192;

    explicit ClipboardStore(std::string directory = defaultDirectory(), size_t limit = 200);
    ~ClipboardStore();

    ClipboardStore(const ClipboardStore &) = delete;
    ClipboardStore &operator=(const ClipboardStore &) = delete;

    static std::string defaultDirectory();

    // Which of the offered types to keep, in order; empty when nothing
    // should be, a password manager's copy for one.
    static std::vector<std::string> formatsToKeep(const std::vector<std::string> &offered);
    // What a payload shows as; false when it holds nothing worth keeping.
    static bool describe(const ClipboardPayload &payload, ClipboardEntry *entry);
    static std::string keyFor(const ClipboardPayload &payload);

    // The index has been read.
    bool isLoaded() const { return m_loaded; }
    const std::vector<ClipboardEntry> &entries() const { return m_entries; }
    int indexOf(const std::string &key) const;

    // Records a copy; false when it is not kept.
    bool add(const ClipboardPayload &payload, std::optional<int64_t> copied = std::nullopt);
    void moveToTop(const std::string &key);
    void remove(const std::string &key);
    void clear();

    // Reads the entry's body; an empty payload when it cannot.
    void payload(const std::string &key, std::function<void(ClipboardPayload)> done);
    static ClipboardPayload readBody(const std::string &path);
    std::string bodyPath(const std::string &key) const;

    // The entries changed.
    base::Signal<> changed;

private:
    void save();

    std::string m_directory;
    size_t m_limit;
    bool m_loaded = false;
    std::vector<ClipboardEntry> m_entries;
    // Bodies on their way to the disk.
    std::set<std::string> m_writing;
    base::Job m_load;
    std::vector<base::Job> m_jobs;
};

} // namespace Lingmo::Shell

