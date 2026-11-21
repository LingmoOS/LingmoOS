// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace ArchiveManager {
enum class Error { None, Canceled, InvalidArchive, InvalidPath, UnsupportedEntry, Exists, Io, Limit, Password };
enum class Format { Zip, Tar, TarGzip, TarXz };
struct Entry {
    std::string path;
    uint64_t size = 0;
    bool directory = false;
    bool encrypted = false;
    bool unsafe = false;
};
struct Progress { uint64_t bytes = 0; size_t entries = 0; std::string path; };
struct Options {
    std::string password;
    std::shared_ptr<std::atomic_bool> canceled;
    std::function<void(const Progress&)> progress;
    uint64_t maxBytes = uint64_t(20) * 1024 * 1024 * 1024;
    size_t maxEntries = 100000;
};
struct Result {
    Error error = Error::None;
    std::string detail;
    std::string output;
    std::string format;
    std::vector<Entry> entries;
    uint64_t totalBytes = 0;
    bool ok() const { return error == Error::None; }
};
// No extraction into an existing directory: commit a private staging directory
// as a new child of destinationParent, without replacing anything.
Result inspect(const std::string& archive, const Options& options = {});
Result verify(const std::string& archive, const Options& options = {});
Result extract(const std::string& archive, const std::string& destinationParent,
               const std::vector<std::string>& selected = {}, const Options& options = {});
Result create(const std::vector<std::string>& sources, const std::string& destination,
              Format format, const Options& options = {});
bool safeArchivePath(const std::string& name, std::string& normalized);
std::string defaultExtractionName(const std::string& archive);
std::string formatExtension(Format format);
// Child directories are synthesized when archive headers omit them.
std::vector<Entry> children(const std::vector<Entry>& entries, const std::string& directory,
                            const std::string& query = {});
}
