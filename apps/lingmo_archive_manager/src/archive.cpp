// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivecore.h"
#include <archive.h>
#include <archive_entry.h>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <limits>
#include <locale.h>
#include <map>
#include <set>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <utility>
namespace ArchiveManager {
namespace {
namespace fs = std::filesystem;
struct Failure { Error code; std::string message; };
[[noreturn]] void fail(Error code, const std::string& message) { throw Failure{code, message}; }
// libarchive converts filenames through the calling thread's locale. Keep
// UTF-8 handling local to this worker rather than changing the process locale.
struct FilenameLocale {
    locale_t value = nullptr;
    locale_t previous = nullptr;
    FilenameLocale() {
        value = ::newlocale(LC_CTYPE_MASK, "C.UTF-8", nullptr);
        if (!value) fail(Error::Io, "UTF-8 filename locale is unavailable");
        previous = ::uselocale(value);
        if (!previous) { ::freelocale(value); value = nullptr; fail(Error::Io, "Cannot set filename locale"); }
    }
    ~FilenameLocale() { if (value) { ::uselocale(previous); ::freelocale(value); } }
};
struct Fd {
    int value = -1;
    explicit Fd(int fd = -1) : value(fd) {}
    ~Fd() { if (value >= 0) ::close(value); }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept : value(std::exchange(other.value, -1)) {}
    Fd& operator=(Fd&& other) noexcept {
        if (this != &other) { if (value >= 0) ::close(value); value = std::exchange(other.value, -1); }
        return *this;
    }
};
struct Temporary {
    fs::path path;
    bool directory = false;
    ~Temporary() {
        if (path.empty()) return;
        std::error_code ec;
        if (directory) fs::remove_all(path, ec); else fs::remove(path, ec);
    }
};
using Reader = std::unique_ptr<struct archive, decltype(&archive_read_free)>;
using Writer = std::unique_ptr<struct archive, decltype(&archive_write_free)>;
using Header = std::unique_ptr<archive_entry, decltype(&archive_entry_free)>;
std::string systemError() { return std::strerror(errno); }
std::string archiveError(struct archive* reader) {
    const char* error = archive_error_string(reader);
    return error ? error : "Archive operation failed";
}
void checkCanceled(const Options& options) {
    if (options.canceled && options.canceled->load()) fail(Error::Canceled, "Operation canceled");
}
void checkedAdd(uint64_t& total, uint64_t value, const Options& options) {
    if (total > options.maxBytes || value > options.maxBytes - total) fail(Error::Limit, "Uncompressed size limit exceeded");
    total += value;
}
void notify(const Options& options, uint64_t bytes, size_t entries, const std::string& path) {
    checkCanceled(options);
    if (options.progress) options.progress({bytes, entries, path});
}
Error readError(struct archive* reader) {
    const auto text = archiveError(reader);
    return archive_read_has_encrypted_entries(reader) > 0 || text.find("Passphrase") != std::string::npos
        || text.find("passphrase") != std::string::npos || text.find("password") != std::string::npos
        ? Error::Password : Error::InvalidArchive;
}
Reader readerFor(const std::string& path, const Options& options) {
    Reader reader(archive_read_new(), archive_read_free);
    if (!reader) fail(Error::Io, "Cannot allocate archive reader");
    archive_read_support_filter_all(reader.get());
    // Deliberately avoid mtree and formats which refer to external disk paths.
    archive_read_support_format_zip(reader.get());
    archive_read_support_format_tar(reader.get());
    archive_read_support_format_7zip(reader.get());
    archive_read_support_format_rar(reader.get());
    archive_read_support_format_rar5(reader.get());
    archive_read_support_format_empty(reader.get());
    archive_read_support_format_raw(reader.get());
    if (!options.password.empty()) archive_read_add_passphrase(reader.get(), options.password.c_str());
    if (archive_read_open_filename(reader.get(), path.c_str(), 64 * 1024) != ARCHIVE_OK)
        fail(readError(reader.get()), archiveError(reader.get()));
    return reader;
}
std::string rawName(const std::string& archivePath) {
    auto name = fs::path(archivePath).filename().string();
    for (const auto* extension : {".gzip", ".gz", ".xz", ".bz2", ".zst", ".lz4", ".lzma"}) {
        if (name.ends_with(extension)) { name.resize(name.size() - std::strlen(extension)); break; }
    }
    return name.empty() ? "data" : name;
}
Entry entryFor(struct archive* reader, archive_entry* header, const std::string& path) {
    const char* name = archive_entry_pathname_utf8(header);
    if (!name) name = archive_entry_pathname(header);
    std::string source = name ? name : "";
    if ((archive_format(reader) & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_RAW) {
        if (archive_filter_code(reader, 0) == ARCHIVE_FILTER_NONE) fail(Error::InvalidArchive, "Not a supported archive");
        source = rawName(path);
    }
    Entry entry;
    entry.unsafe = !safeArchivePath(source, entry.path);
    if (entry.unsafe) entry.path = source;
    const auto type = archive_entry_filetype(header);
    entry.directory = type == AE_IFDIR;
    if (type != AE_IFREG && type != AE_IFDIR) entry.unsafe = true;
    if (archive_entry_symlink(header) || archive_entry_hardlink(header)) entry.unsafe = true;
    if (archive_entry_size_is_set(header)) {
        const auto size = archive_entry_size(header);
        if (size < 0) fail(Error::InvalidArchive, "Negative archive entry size");
        entry.size = static_cast<uint64_t>(size);
    }
    entry.encrypted = archive_entry_is_encrypted(header) > 0;
    return entry;
}
bool selectedEntry(const std::string& path, const std::vector<std::string>& selected) {
    if (selected.empty()) return true;
    for (const auto& prefix : selected)
        if (path == prefix || path.starts_with(prefix + '/')) return true;
    return false;
}
Fd makeDirectory(int root, const fs::path& path) {
    Fd current(::dup(root));
    if (current.value < 0) fail(Error::Io, systemError());
    for (const auto& component : path) {
        const auto part = component.string();
        if (part.empty() || part == ".") continue;
        if (::mkdirat(current.value, part.c_str(), 0700) != 0 && errno != EEXIST) fail(Error::Io, systemError());
        Fd next(::openat(current.value, part.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        if (next.value < 0) fail(Error::InvalidPath, "Archive directory conflicts with another entry");
        current = std::move(next);
    }
    return current;
}
void writeAll(int fd, const char* data, size_t size) {
    while (size) {
        const auto written = ::write(fd, data, size);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) fail(Error::Io, systemError());
        data += written; size -= static_cast<size_t>(written);
    }
}
void consume(struct archive* reader, int output, uint64_t& bytes, size_t count,
             const std::string& path, const Options& options) {
    std::array<char, 65536> buffer;
    uint64_t lastReport = bytes;
    for (;;) {
        checkCanceled(options);
        const auto length = archive_read_data(reader, buffer.data(), buffer.size());
        if (length == 0) break;
        if (length < 0) fail(readError(reader), archiveError(reader));
        checkedAdd(bytes, static_cast<uint64_t>(length), options);
        if (output >= 0) writeAll(output, buffer.data(), static_cast<size_t>(length));
        if (bytes - lastReport >= 1024 * 1024) { notify(options, bytes, count, path); lastReport = bytes; }
    }
}
fs::path parentFor(const fs::path& destination) {
    std::error_code ec;
    const auto parent = fs::canonical(destination, ec);
    if (ec || !fs::is_directory(parent, ec) || ec) fail(Error::Io, "Destination directory is unavailable");
    return parent;
}
bool commitNoReplace(const fs::path& from, const fs::path& to) {
    if (::syscall(SYS_renameat2, AT_FDCWD, from.c_str(), AT_FDCWD, to.c_str(), 1 /* RENAME_NOREPLACE */) == 0) return true;
    if (errno == EEXIST) return false;
    fail(Error::Io, systemError());
}
template<class Work> Result guarded(Work work) {
    try { FilenameLocale locale; return work(); }
    catch (const Failure& error) { Result result; result.error = error.code; result.detail = error.message; return result; }
    catch (const std::exception& error) { Result result; result.error = Error::Io; result.detail = error.what(); return result; }
}
Result scan(const std::string& path, const Options& options, bool data) {
    return guarded([&] {
        checkCanceled(options);
        auto reader = readerFor(path, options);
        Result result;
        archive_entry* header = nullptr;
        size_t count = 0;
        uint64_t bytes = 0;
        int status;
        while ((status = archive_read_next_header(reader.get(), &header)) == ARCHIVE_OK) {
            checkCanceled(options);
            if (++count > options.maxEntries) fail(Error::Limit, "Archive entry limit exceeded");
            auto entry = entryFor(reader.get(), header, path);
            checkedAdd(result.totalBytes, entry.size, options);
            const bool raw = (archive_format(reader.get()) & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_RAW;
            if (data || raw) {
                consume(reader.get(), -1, bytes, count, entry.path, options);
                if (raw) { entry.size = bytes; result.totalBytes = bytes; }
            }
            else if (archive_read_data_skip(reader.get()) < ARCHIVE_OK) fail(readError(reader.get()), archiveError(reader.get()));
            if (!entry.path.empty()) result.entries.push_back(std::move(entry));
            if (count % 128 == 0) notify(options, bytes, count, result.entries.empty() ? "" : result.entries.back().path);
        }
        if (status != ARCHIVE_EOF) fail(readError(reader.get()), archiveError(reader.get()));
        if (const char* format = archive_format_name(reader.get())) result.format = format;
        if (data) result.totalBytes = bytes;
        result.output = path;
        notify(options, bytes, count, {});
        return result;
    });
}
struct Source { fs::path disk; std::string name; uint64_t size; bool directory; mode_t mode; };
}

bool safeArchivePath(const std::string& name, std::string& normalized)
{
    normalized.clear();
    if (name.empty() || name.size() > 4096 || name.front() == '/' || name.find('\\') != std::string::npos
        || name.find(':') != std::string::npos) return false;
    for (unsigned char c : name) if (c < 32 || c == 127) return false;
    size_t depth = 0;
    for (const auto& component : std::filesystem::path(name)) {
        const auto part = component.string();
        if (part == ".." || part.size() > 255 || ++depth > 128) return false;
        if (part.empty() || part == ".") continue;
        if (!normalized.empty()) normalized += '/';
        normalized += part;
    }
    return true;
}
std::string defaultExtractionName(const std::string& path)
{
    auto name = std::filesystem::path(path).filename().string();
    for (const auto* ext : {".tar.gz", ".tar.xz", ".tar.bz2", ".tar.zst", ".tgz", ".txz", ".zip", ".7z", ".rar", ".tar", ".gz", ".xz", ".bz2", ".zst"})
        if (name.ends_with(ext)) { name.resize(name.size() - std::strlen(ext)); break; }
    std::string safe;
    if (name.empty() || !safeArchivePath(name, safe) || safe.empty()) name = "Archive";
    if (name.size() > 180) name = "Archive";
    return name + "-extracted";
}
std::string formatExtension(Format format)
{
    switch (format) { case Format::Zip: return ".zip"; case Format::Tar: return ".tar";
    case Format::TarGzip: return ".tar.gz"; case Format::TarXz: return ".tar.xz"; }
    return ".zip";
}
Result inspect(const std::string& path, const Options& options) { return scan(path, options, false); }
Result verify(const std::string& path, const Options& options) { return scan(path, options, true); }

Result extract(const std::string& path, const std::string& destinationParent,
               const std::vector<std::string>& selected, const Options& options)
{
    return guarded([&] {
        checkCanceled(options);
        std::vector<std::string> selection;
        for (const auto& name : selected) {
            std::string clean;
            if (!safeArchivePath(name, clean) || clean.empty()) fail(Error::InvalidPath, "Invalid selected path");
            selection.push_back(std::move(clean));
        }
        const auto parent = parentFor(destinationParent);
        auto pattern = (parent / ".lingmo-extract-XXXXXX").string();
        if (!::mkdtemp(pattern.data())) fail(Error::Io, systemError());
        Temporary temporary{pattern, true};
        Fd root(::open(pattern.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        if (root.value < 0) fail(Error::Io, systemError());
        auto reader = readerFor(path, options);
        archive_entry* header = nullptr;
        size_t count = 0, extracted = 0;
        uint64_t bytes = 0;
        std::set<std::string> files;
        int status;
        while ((status = archive_read_next_header(reader.get(), &header)) == ARCHIVE_OK) {
            checkCanceled(options);
            if (++count > options.maxEntries) fail(Error::Limit, "Archive entry limit exceeded");
            auto entry = entryFor(reader.get(), header, path);
            if (entry.path.empty() && entry.directory) continue;
            if (!selectedEntry(entry.path, selection)) {
                if (archive_read_data_skip(reader.get()) < ARCHIVE_OK) fail(readError(reader.get()), archiveError(reader.get()));
                continue;
            }
            if (entry.unsafe || entry.path.empty()) fail(Error::UnsupportedEntry, "Unsafe path, link or special entry: " + entry.path);
            if (entry.size > options.maxBytes - bytes) fail(Error::Limit, "Uncompressed size limit exceeded");
            const fs::path relative(entry.path);
            if (entry.directory) {
                auto folder = makeDirectory(root.value, relative);
                consume(reader.get(), -1, bytes, extracted, entry.path, options);
            }
            else {
                if (!files.insert(entry.path).second) fail(Error::InvalidPath, "Duplicate file entry: " + entry.path);
                auto folder = makeDirectory(root.value, relative.parent_path());
                Fd output(::openat(folder.value, relative.filename().c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
                if (output.value < 0) fail(Error::InvalidPath, "Conflicting archive entry: " + entry.path);
                consume(reader.get(), output.value, bytes, extracted, entry.path, options);
                const mode_t permissions = archive_entry_perm(header) & 0111 ? 0700 : 0600;
                if (::fchmod(output.value, permissions) != 0 || ::fsync(output.value) != 0) fail(Error::Io, systemError());
            }
            ++extracted;
            if (extracted % 32 == 0) notify(options, bytes, extracted, entry.path);
        }
        if (status != ARCHIVE_EOF) fail(readError(reader.get()), archiveError(reader.get()));
        if (!selection.empty() && !extracted) fail(Error::InvalidPath, "Selected entries are no longer present");
        checkCanceled(options);
        const auto base = defaultExtractionName(path);
        fs::path target;
        for (unsigned suffix = 0; suffix < 10000; ++suffix) {
            target = parent / (base + (suffix ? "-" + std::to_string(suffix) : ""));
            if (commitNoReplace(temporary.path, target)) { temporary.path.clear(); break; }
        }
        if (!temporary.path.empty()) fail(Error::Exists, "Cannot allocate extraction folder");
        Result result; result.output = target.string(); result.totalBytes = bytes;
        return result;
    });
}

Result create(const std::vector<std::string>& sources, const std::string& destination,
              Format format, const Options& options)
{
    return guarded([&] {
        checkCanceled(options);
        if (sources.empty()) fail(Error::InvalidPath, "No source files selected");
        if (!options.password.empty()) fail(Error::Password, "Creating encrypted archives is not supported");
        const fs::path requested = fs::absolute(destination).lexically_normal();
        const auto parent = parentFor(requested.parent_path());
        const fs::path target = parent / requested.filename();
        struct stat st{};
        if (::lstat(target.c_str(), &st) == 0) fail(Error::Exists, "Destination already exists");
        std::vector<Source> entries;
        std::set<std::string> names;
        uint64_t total = 0;
        auto collect = [&](const fs::path& disk, const std::string& name) {
            checkCanceled(options);
            std::string clean;
            if (!safeArchivePath(name, clean) || clean.empty()) fail(Error::InvalidPath, "Invalid source name: " + name);
            struct stat info{};
            if (::lstat(disk.c_str(), &info) != 0) fail(Error::Io, systemError());
            if (!S_ISREG(info.st_mode) && !S_ISDIR(info.st_mode)) fail(Error::UnsupportedEntry, "Links and special files cannot be added: " + disk.string());
            if (!names.insert(clean).second) fail(Error::InvalidPath, "Duplicate source name: " + clean);
            if (entries.size() >= options.maxEntries) fail(Error::Limit, "Source entry limit exceeded");
            const bool directory = S_ISDIR(info.st_mode);
            const auto size = directory ? uint64_t(0) : static_cast<uint64_t>(info.st_size);
            checkedAdd(total, size, options);
            entries.push_back({disk, clean, size, directory, info.st_mode});
        };
        for (const auto& source : sources) {
            const auto disk = fs::absolute(source).lexically_normal();
            const auto name = disk.filename().string();
            collect(disk, name);
            if (!entries.back().directory) continue;
            const auto canonical = fs::canonical(disk);
            const auto relativeTarget = target.lexically_relative(canonical);
            if (!relativeTarget.empty() && *relativeTarget.begin() != "..")
                fail(Error::InvalidPath, "Save the archive outside the selected source folder");
            for (const auto& child : fs::recursive_directory_iterator(disk))
                collect(child.path(), name + '/' + child.path().lexically_relative(disk).generic_string());
        }
        std::sort(entries.begin(), entries.end(), [](const Source& a, const Source& b) { return a.name < b.name; });
        auto pattern = (parent / ".lingmo-archive-XXXXXX").string();
        Fd output(::mkstemp(pattern.data()));
        if (output.value < 0) fail(Error::Io, systemError());
        Temporary temporary{pattern, false};
        Writer writer(archive_write_new(), archive_write_free);
        if (!writer) fail(Error::Io, "Cannot allocate archive writer");
        int status = format == Format::Zip ? archive_write_set_format_zip(writer.get()) : archive_write_set_format_pax_restricted(writer.get());
        if (status != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
        if (format == Format::TarGzip && archive_write_add_filter_gzip(writer.get()) != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
        if (format == Format::TarXz && archive_write_add_filter_xz(writer.get()) != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
        if (archive_write_open_fd(writer.get(), output.value) != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
        uint64_t bytes = 0;
        size_t count = 0;
        std::array<char, 65536> buffer;
        for (const auto& entry : entries) {
            checkCanceled(options);
            Header header(archive_entry_new(), archive_entry_free);
            archive_entry_set_pathname_utf8(header.get(), entry.name.c_str());
            archive_entry_set_filetype(header.get(), entry.directory ? AE_IFDIR : AE_IFREG);
            archive_entry_set_perm(header.get(), entry.directory || (entry.mode & 0111) ? 0755 : 0644);
            archive_entry_set_size(header.get(), static_cast<la_int64_t>(entry.size));
            if (archive_write_header(writer.get(), header.get()) != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
            if (!entry.directory) {
                Fd input(::open(entry.disk.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
                struct stat info{};
                if (input.value < 0 || ::fstat(input.value, &info) != 0 || !S_ISREG(info.st_mode)
                    || static_cast<uint64_t>(info.st_size) != entry.size) fail(Error::Io, "Source file changed: " + entry.name);
                uint64_t remaining = entry.size;
                while (remaining) {
                    checkCanceled(options);
                    const auto length = ::read(input.value, buffer.data(), std::min<uint64_t>(buffer.size(), remaining));
                    if (length < 0 && errno == EINTR) continue;
                    if (length <= 0) fail(Error::Io, "Cannot read source: " + entry.name);
                    size_t offset = 0;
                    while (offset < static_cast<size_t>(length)) {
                        const auto written = archive_write_data(writer.get(), buffer.data() + offset, static_cast<size_t>(length) - offset);
                        if (written <= 0) fail(Error::Io, archiveError(writer.get()));
                        offset += static_cast<size_t>(written);
                    }
                    remaining -= length; bytes += length;
                    if (bytes % (1024 * 1024) < buffer.size()) notify(options, bytes, count, entry.name);
                }
            }
            if (archive_write_finish_entry(writer.get()) != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
            ++count;
            if (count % 32 == 0) notify(options, bytes, count, entry.name);
        }
        if (archive_write_close(writer.get()) != ARCHIVE_OK) fail(Error::Io, archiveError(writer.get()));
        if (::fsync(output.value) != 0) fail(Error::Io, systemError());
        checkCanceled(options);
        if (!commitNoReplace(temporary.path, target)) fail(Error::Exists, "Destination already exists");
        temporary.path.clear();
        Result result; result.output = target.string(); result.totalBytes = bytes;
        return result;
    });
}
std::vector<Entry> children(const std::vector<Entry>& entries, const std::string& directory, const std::string& query)
{
    const auto prefix = directory.empty() ? std::string() : directory + '/';
    std::map<std::string, Entry> result;
    std::string filter = query;
    std::transform(filter.begin(), filter.end(), filter.begin(), [](unsigned char c) { return std::tolower(c); });
    for (const auto& entry : entries) {
        if (!entry.path.starts_with(prefix)) continue;
        const auto remainder = entry.path.substr(prefix.size());
        if (remainder.empty()) continue;
        if (!filter.empty()) {
            auto lower = remainder;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
            if (lower.find(filter) == std::string::npos) continue;
            result[entry.path] = entry;
        } else {
            const auto slash = remainder.find('/');
            if (slash == std::string::npos) result[entry.path] = entry;
            else {
                const auto name = prefix + remainder.substr(0, slash);
                if (!result.count(name)) result[name] = {name, 0, true, false, entry.unsafe};
            }
        }
    }
    std::vector<Entry> rows;
    for (auto& [path, entry] : result) rows.push_back(std::move(entry));
    std::sort(rows.begin(), rows.end(), [](const Entry& a, const Entry& b) {
        return a.directory != b.directory ? a.directory > b.directory : a.path < b.path;
    });
    return rows;
}
}
