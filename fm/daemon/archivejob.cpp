#include "archivejob.h"

#include <archive.h>
#include <archive_entry.h>
#include <zip.h>

#include "base/i18n.h"
#include "base/strings.h"
#include "base/uuid.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <regex>

#include <dirent.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

static std::string errorString(int code)
{
    return std::strerror(code);
}

static std::string fileNameOf(const std::string &path)
{
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

static std::string absoluteParent(const std::string &path)
{
    std::error_code error;
    const fs::path absolute = fs::absolute(path, error);
    return absolute.parent_path().string();
}

static bool existsNoFollow(const std::string &path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0;
}

static bool isDirectory(const std::string &path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static bool makePath(const std::string &path)
{
    std::error_code error;
    fs::create_directories(path, error);
    return isDirectory(path);
}

static void removeFile(const std::string &path)
{
    ::unlink(path.c_str());
}

// A rename that never replaces an existing file.
static bool renameNoReplace(const std::string &from, const std::string &to)
{
    return ::renameat2(AT_FDCWD, from.c_str(), AT_FDCWD, to.c_str(), RENAME_NOREPLACE) == 0;
}

static std::string zipError(zip_t *archive)
{
    if (!archive)
        return tr("Unknown ZIP error");

    return zip_strerror(archive);
}

static std::string zipOpenError(int errorCode)
{
    zip_error_t error;
    zip_error_init_with_code(&error, errorCode);
    const std::string message = zip_error_strerror(&error);
    zip_error_fini(&error);
    return message;
}

using ArchiveReader = std::unique_ptr<struct archive, decltype(&archive_read_free)>;

static constexpr const char *kEncryptedRar = trNoop("Password-protected RAR archives are not supported");

static int baseFormat(struct archive *reader)
{
    return archive_format(reader) & ARCHIVE_FORMAT_BASE_MASK;
}

static std::string encryptionUnsupported(struct archive *reader)
{
    switch (baseFormat(reader)) {
    case ARCHIVE_FORMAT_7ZIP:
        return tr("Password-protected 7z archives are not supported");
    case ARCHIVE_FORMAT_RAR:
    case ARCHIVE_FORMAT_RAR_V5:
        return tr(kEncryptedRar);
    case ARCHIVE_FORMAT_ZIP:
        return tr("This kind of ZIP encryption is not supported");
    default:
        return tr("Password-protected archives of this format are not supported");
    }
}

static std::string archiveError(struct archive *reader)
{
    const bool encrypted = archive_read_has_encrypted_entries(reader) > 0;
    // Only ZIP entries are decrypted; the other formats fail on an encrypted
    // entry or header with a message of their own.
    if (encrypted && baseFormat(reader) != ARCHIVE_FORMAT_ZIP)
        return encryptionUnsupported(reader);

    if (const char *message = archive_error_string(reader))
        return message;

    // An encrypted ZIP central directory fails without a message.
    return encrypted ? encryptionUnsupported(reader) : std::string(tr("Unknown archive error"));
}

static ArchiveReader openArchiveReader(const std::vector<std::string> &volumes, archive_passphrase_callback *password,
                                       void *passwordState, std::string *error)
{
    ArchiveReader reader(archive_read_new(), &archive_read_free);
    if (!reader) {
        *error = tr("Could not allocate memory for the archive");
        return reader;
    }

    // Not format_all: formats such as mtree name files elsewhere on disk.
    archive_read_support_format_7zip(reader.get());
    archive_read_support_format_rar(reader.get());
    archive_read_support_format_rar5(reader.get());
    archive_read_support_format_tar(reader.get());
    archive_read_support_format_zip(reader.get());
    archive_read_support_format_empty(reader.get());
    // Raw bids lowest, so it only takes what no archive format claims.
    archive_read_support_format_raw(reader.get());

    archive_read_support_filter_bzip2(reader.get());
    archive_read_support_filter_compress(reader.get());
    archive_read_support_filter_gzip(reader.get());
    archive_read_support_filter_lzip(reader.get());
    archive_read_support_filter_lzma(reader.get());
    archive_read_support_filter_xz(reader.get());
    archive_read_support_filter_zstd(reader.get());

    archive_read_set_passphrase_callback(reader.get(), passwordState, password);

    std::vector<const char *> names;
    for (const std::string &volume : volumes)
        names.push_back(volume.c_str());
    names.push_back(nullptr);

    if (archive_read_open_filenames(reader.get(), names.data(), 64 * 1024) != ARCHIVE_OK) {
        *error = archiveError(reader.get());
        reader.reset();
    }

    return reader;
}

static bool isRegularFile(const std::string &path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

static std::string numbered(unsigned number, size_t width)
{
    std::string digits = std::to_string(number);
    if (digits.size() < width)
        digits.insert(0, width - digits.size(), '0');
    return digits;
}

// The parts of the archive in order, from the first whichever part is named:
// name.partN.rar, name.rar followed by name.r00, name.7z.NNN and name.zip.NNN.
static bool archiveVolumes(const std::string &archivePath, std::vector<std::string> *volumes, std::string *error)
{
    static const std::regex rarPart(R"(^(.*\.part)(\d+)(\.rar)$)", std::regex::icase);
    static const std::regex splitPart(R"(^(.*\.(?:7z|zip)\.)(\d{3,})$)", std::regex::icase);
    static const std::regex rar(R"(^.*\.(r)ar$)", std::regex::icase);
    static constexpr unsigned kMaxVolumes = 10000;

    const auto collect = [volumes](const std::string &prefix, size_t width, const std::string &suffix,
                                   unsigned first) {
        for (unsigned number = first; number < first + kMaxVolumes; ++number) {
            const std::string volume = prefix + numbered(number, width) + suffix;
            if (!isRegularFile(volume))
                break;
            volumes->push_back(volume);
        }
    };

    std::smatch match;
    if (std::regex_match(archivePath, match, rarPart))
        collect(match.str(1), size_t(match.length(2)), match.str(3), 1);
    else if (std::regex_match(archivePath, match, splitPart))
        collect(match.str(1), size_t(match.length(2)), {}, 1);
    else {
        volumes->push_back(archivePath);
        if (std::regex_match(archivePath, match, rar))
            collect(archivePath.substr(0, archivePath.size() - 3) + match.str(1), 2, {}, 0);
        return true;
    }

    if (volumes->empty()) {
        *error = tr("The first part of this multi-part archive is missing");
        return false;
    }
    return true;
}

static bool readVint(const std::vector<unsigned char> &data, size_t *offset, uint64_t *value)
{
    *value = 0;
    for (int shift = 0; shift < 64 && *offset < data.size(); shift += 7) {
        const unsigned char byte = data[(*offset)++];
        *value |= uint64_t(byte & 0x7f) << shift;
        if (!(byte & 0x80))
            return true;
    }
    return false;
}

// libarchive 3.7 reads a RAR5 entry past its encryption record as if its data
// were plain, and fails on it later with a decoding error; the headers tell.
static bool isEncryptedRar5(const std::string &archivePath)
{
    static constexpr unsigned char kSignature[] = {'R', 'a', 'r', '!', 0x1a, 0x07, 0x01, 0x00};
    static constexpr uint64_t kMaxHeaderSize = 2 * 1024 * 1024;
    enum : uint64_t {
        FileHeader = 2,
        ServiceHeader = 3,
        EncryptionHeader = 4,
        EndHeader = 5,
        EncryptionRecord = 1,
    };

    const int fd = ::open(archivePath.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return false;
    struct Closer {
        int fd;
        ~Closer() { ::close(fd); }
    } closer {fd};

    unsigned char signature[sizeof(kSignature)];
    if (::pread(fd, signature, sizeof(signature), 0) != ssize_t(sizeof(signature))
        || std::memcmp(signature, kSignature, sizeof(kSignature)) != 0)
        return false;

    uint64_t position = sizeof(kSignature);
    for (int block = 0; block < 100000; ++block) {
        // The CRC32, then the header size as a variable-length integer.
        std::vector<unsigned char> prefix(4 + 3);
        const ssize_t prefixRead = ::pread(fd, prefix.data(), prefix.size(), off_t(position));
        if (prefixRead < 5)
            return false;
        prefix.resize(size_t(prefixRead));
        size_t offset = 4;
        uint64_t headerSize = 0;
        if (!readVint(prefix, &offset, &headerSize) || headerSize == 0 || headerSize > kMaxHeaderSize)
            return false;

        const uint64_t headerStart = position + offset;
        std::vector<unsigned char> header(headerSize);
        if (::pread(fd, header.data(), header.size(), off_t(headerStart)) != ssize_t(header.size()))
            return false;

        offset = 0;
        uint64_t type = 0;
        uint64_t flags = 0;
        uint64_t extraSize = 0;
        uint64_t dataSize = 0;
        if (!readVint(header, &offset, &type) || !readVint(header, &offset, &flags))
            return false;
        if ((flags & 0x0001) && !readVint(header, &offset, &extraSize))
            return false;
        if ((flags & 0x0002) && !readVint(header, &offset, &dataSize))
            return false;

        if (type == EncryptionHeader)
            return true;
        if (type == EndHeader)
            return false;

        if ((type == FileHeader || type == ServiceHeader) && extraSize > 0 && extraSize <= headerSize) {
            // The extra area ends the header: records of a size, then a type.
            size_t record = size_t(headerSize - extraSize);
            while (record < header.size()) {
                uint64_t recordSize = 0;
                uint64_t recordType = 0;
                if (!readVint(header, &record, &recordSize) || recordSize == 0 || recordSize > header.size())
                    return false;
                const size_t next = record + size_t(recordSize);
                if (!readVint(header, &record, &recordType))
                    return false;
                if (recordType == EncryptionRecord)
                    return true;
                record = next;
            }
        }

        position = headerStart + headerSize + dataSize;
    }

    return false;
}

static std::string entryPathName(struct archive_entry *entry)
{
    if (const char *name = archive_entry_pathname_utf8(entry))
        return name;

    const char *name = archive_entry_pathname(entry);
    return name ? std::string(name) : std::string();
}

static std::string entryLinkTarget(struct archive_entry *entry)
{
    if (const char *target = archive_entry_symlink_utf8(entry))
        return target;

    const char *target = archive_entry_symlink(entry);
    return target ? std::string(target) : std::string();
}

static struct timespec entryModified(struct archive_entry *entry)
{
    struct timespec modified;
    modified.tv_sec = archive_entry_mtime(entry);
    modified.tv_nsec = archive_entry_mtime_nsec(entry);
    return modified;
}

static std::string temporaryPath(const std::string &parentPath, const std::string &prefix)
{
    return parentPath + "/." + prefix + '-' + Lingmo::base::Uuid::generate().toString();
}

static bool isDirectoryEntry(const std::string &name)
{
    return name.ends_with('/');
}

static bool hasSymbolicLinkParent(const std::string &rootPath, const std::string &relativePath)
{
    const auto components = Lingmo::base::strings::split(relativePath, '/',
                                                            Lingmo::base::strings::SplitBehavior::SkipEmptyParts);
    std::string currentPath = rootPath;

    for (size_t index = 0; index + 1 < components.size(); ++index) {
        currentPath += '/' + components[index];
        struct stat st;
        if (::lstat(currentPath.c_str(), &st) != 0)
            break;
        if (S_ISLNK(st.st_mode))
            return true;
    }

    return false;
}

static std::string topLevelArchiveName(const std::string &archivePath)
{
    const size_t separator = archivePath.find('/');
    return separator == std::string::npos ? archivePath : archivePath.substr(0, separator);
}

ArchiveJob::ArchiveJob(Operation operation, std::vector<std::string> inputs, std::string outputPath)
    : m_operation(operation)
    , m_inputs(std::move(inputs))
    , m_outputPath(std::move(outputPath))
    , m_cancelRequested(false)
{
}

ArchiveJob::~ArchiveJob()
{
    cancel();
    if (m_thread.joinable())
        m_thread.join();
}

void ArchiveJob::start()
{
    m_thread = std::thread([this] { run(); });
}

void ArchiveJob::cancel()
{
    m_cancelRequested.store(true);
    std::lock_guard lock(m_passwordMutex);
    m_passwordCondition.notify_all();
}

bool ArchiveJob::answerPassword(uint32_t request, std::string password)
{
    std::lock_guard lock(m_passwordMutex);
    if (request == 0 || request != m_passwordRequest || m_answer)
        return false;

    m_answer = std::move(password);
    m_passwordCondition.notify_all();
    return true;
}

bool ArchiveJob::isCanceled() const
{
    return m_cancelRequested.load();
}

// libarchive asks again, before it returns, while the passwords it has do
// not open the entry; a null answer fails the entry.
const char *ArchiveJob::requestPassword()
{
    std::unique_lock lock(m_passwordMutex);
    if (isCanceled())
        return nullptr;

    const uint32_t request = ++m_lastPasswordRequest;
    m_passwordRequest = request;
    m_answer.reset();
    const bool retry = m_passwordsGiven > 0;
    lock.unlock();
    if (onPasswordRequested)
        onPasswordRequested(request, retry);
    lock.lock();

    m_passwordCondition.wait(lock, [this] { return m_answer.has_value() || isCanceled(); });
    m_passwordRequest = 0;
    if (isCanceled()) {
        m_answer.reset();
        return nullptr;
    }

    m_password = std::move(*m_answer);
    m_answer.reset();
    ++m_passwordsGiven;
    m_passwordPending = true;
    return m_password.c_str();
}

void ArchiveJob::settlePassword()
{
    if (!m_passwordPending)
        return;

    m_passwordPending = false;
    if (onPasswordAccepted)
        onPasswordAccepted();
}

void ArchiveJob::progressChanged(int progress)
{
    if (onProgress)
        onProgress(progress);
}

void ArchiveJob::currentFileChanged(const std::string &fileName)
{
    if (onCurrentFile)
        onCurrentFile(fileName);
}

void ArchiveJob::run()
{
    std::string error;
    const bool success = m_operation == Compress ? compress(&error) : extract(&error);
    // Once the final rename has succeeded, the operation is committed. A
    // cancellation arriving in the tiny window before this function reads
    // the flag must not turn a completed operation into a false cancellation.
    const bool canceled = !success && isCanceled();

    if (onCompleted)
        onCompleted(success && !canceled, canceled, error, success && !canceled ? m_outputPath : std::string());
}

void ArchiveJob::emitProgress(uint64_t processed, uint64_t total)
{
    if (total == 0) {
        progressChanged(processed == 0 ? 0 : 100);
        return;
    }

    const double ratio = static_cast<double>(processed) / static_cast<double>(total);
    const int percent = std::clamp(static_cast<int>(ratio * 100.0), 0, 100);
    // Extraction reports every 64 KiB, and each report becomes a D-Bus
    // broadcast in the daemon; only a changed percentage is worth one.
    if (percent == m_lastPercent)
        return;
    m_lastPercent = percent;
    progressChanged(percent);
}

bool ArchiveJob::collectCompressionEntries(const std::string &sourcePath, const std::string &archivePath,
                                           std::vector<CompressionEntry> *entries, std::string *error) const
{
    if (isCanceled())
        return false;

    struct stat linkStat;
    if (::lstat(sourcePath.c_str(), &linkStat) != 0) {
        *error = format(tr("The source no longer exists: {path}"), {{"path", sourcePath}});
        return false;
    }

    if (S_ISLNK(linkStat.st_mode)) {
        char target[16 * 1024];
        const ssize_t length = ::readlink(sourcePath.c_str(), target, sizeof(target));
        if (length <= 0) {
            *error = format(tr("Could not read symbolic link: {path}"), {{"path", sourcePath}});
            return false;
        }

        entries->push_back({sourcePath, archivePath, false, 0, true, std::string(target, size_t(length))});
        return true;
    }

    if (S_ISDIR(linkStat.st_mode)) {
        const std::string directoryArchivePath = archivePath.ends_with('/') ? archivePath : archivePath + '/';
        entries->push_back({sourcePath, directoryArchivePath, true, 0, false, std::string()});

        std::vector<std::pair<bool, std::string>> children;
        if (DIR *directory = ::opendir(sourcePath.c_str())) {
            while (dirent *entry = ::readdir(directory)) {
                const std::string name = entry->d_name;
                if (name == "." || name == "..")
                    continue;
                children.push_back({!isDirectory(sourcePath + '/' + name), name});
            }
            ::closedir(directory);
        }
        // Folders first, then by name.
        std::sort(children.begin(), children.end());

        for (const auto &[file, name] : children) {
            if (!collectCompressionEntries(sourcePath + '/' + name, directoryArchivePath + name, entries, error))
                return false;
        }

        return true;
    }

    if (!S_ISREG(linkStat.st_mode)) {
        *error = format(tr("The source is not a regular file: {path}"), {{"path", sourcePath}});
        return false;
    }

    entries->push_back({sourcePath, archivePath, false, uint64_t(linkStat.st_size), false, std::string()});
    return true;
}

bool ArchiveJob::compress(std::string *error)
{
    if (m_inputs.empty()) {
        *error = tr("No files were selected");
        return false;
    }

    std::vector<CompressionEntry> entries;
    for (const std::string &input : m_inputs) {
        if (!collectCompressionEntries(input, fileNameOf(input), &entries, error))
            return false;
    }

    if (isCanceled())
        return false;

    const std::string parentPath = absoluteParent(m_outputPath);
    if (!isDirectory(parentPath)) {
        *error = format(tr("The destination folder does not exist: {path}"), {{"path", parentPath}});
        return false;
    }

    const std::string tempPath = temporaryPath(parentPath, "lingmo-archive");
    int openError = 0;
    zip_t *archive = zip_open(tempPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &openError);
    if (!archive) {
        removeFile(tempPath);
        *error = zipOpenError(openError);
        return false;
    }

    const auto fail = [&](std::string message) {
        *error = std::move(message);
        zip_discard(archive);
        removeFile(tempPath);
        return false;
    };

    if (zip_register_progress_callback_with_state(archive, 0.01, &ArchiveJob::zipProgressCallback, nullptr, this)
                != 0
        || zip_register_cancel_callback_with_state(archive, &ArchiveJob::zipCancelCallback, nullptr, this) != 0)
        return fail(zipError(archive));

    progressChanged(0);
    for (const CompressionEntry &entry : entries) {
        if (isCanceled())
            return fail({});

        // Keep the progress UI focused on the selected item. A folder may
        // contain many entries, but its internal archive paths are not useful
        // to show while the archive is being created.
        currentFileChanged(topLevelArchiveName(entry.archivePath));

        if (entry.directory) {
            if (zip_dir_add(archive, entry.archivePath.c_str(), ZIP_FL_ENC_UTF_8) < 0)
                return fail(zipError(archive));
            continue;
        }

        zip_source_t *source = nullptr;
        if (entry.symbolicLink) {
            const std::string &linkTarget = entry.linkTarget;
            const size_t bufferSize = std::max<size_t>(linkTarget.size(), 1);
            char *buffer = static_cast<char *>(std::malloc(bufferSize));
            if (!buffer)
                return fail(tr("Could not allocate memory for symbolic link"));

            std::memcpy(buffer, linkTarget.data(), linkTarget.size());
            source = zip_source_buffer(archive, buffer, linkTarget.size(), 1);
            if (!source)
                std::free(buffer);
        } else {
            source = zip_source_file(archive, entry.sourcePath.c_str(), 0, -1);
        }

        if (!source)
            return fail(zipError(archive));

        const zip_int64_t entryIndex = zip_file_add(archive, entry.archivePath.c_str(), source, ZIP_FL_ENC_UTF_8);
        if (entryIndex < 0) {
            zip_source_free(source);
            return fail(zipError(archive));
        }

        // A fast, low-level DEFLATE profile: quick archives without an
        // expensive maximum-compression pass.
        if (zip_set_file_compression(archive, static_cast<zip_uint64_t>(entryIndex), ZIP_CM_DEFLATE, 1) != 0)
            return fail(zipError(archive));

        if (entry.symbolicLink
            && zip_file_set_external_attributes(archive, static_cast<zip_uint64_t>(entryIndex), 0, ZIP_OPSYS_UNIX,
                                                static_cast<zip_uint32_t>(S_IFLNK | 0777) << 16)
                    != 0)
            return fail(zipError(archive));
    }

    if (isCanceled())
        return fail({});

    if (zip_close(archive) != 0)
        return fail(zipError(archive));

    if (isCanceled() || !renameNoReplace(tempPath, m_outputPath)) {
        removeFile(tempPath);
        if (!isCanceled())
            *error = format(tr("Could not create {path}"), {{"path", m_outputPath}});
        return false;
    }

    progressChanged(100);
    return true;
}

bool ArchiveJob::archiveEntryPath(const std::string &archiveName, std::string *relativePath, bool *directory) const
{
    std::string name = archiveName;
    std::replace(name.begin(), name.end(), '\\', '/');

    *directory = isDirectoryEntry(name);
    while (name.ends_with('/'))
        name.pop_back();

    if (name.empty()) {
        relativePath->clear();
        return true;
    }

    if (name.starts_with('/') || (name.size() >= 2 && name[1] == ':') || name.find('\0') != std::string::npos)
        return false;

    std::vector<std::string> kept;
    for (const std::string &component :
         Lingmo::base::strings::split(name, '/', Lingmo::base::strings::SplitBehavior::SkipEmptyParts)) {
        if (component == "..")
            return false;
        if (component != ".")
            kept.push_back(component);
    }

    // Tar archives made from "." begin with a "./" entry for the root.
    *relativePath = Lingmo::base::strings::join(kept, "/");
    return true;
}

bool ArchiveJob::extractEntryData(struct archive *reader, int fd, uint64_t total, std::string *error)
{
    char buffer[64 * 1024];
    while (true) {
        if (isCanceled())
            return false;

        const la_ssize_t bytesRead = archive_read_data(reader, buffer, sizeof(buffer));
        if (bytesRead < 0) {
            *error = archiveError(reader);
            return false;
        }

        settlePassword();

        if (bytesRead == 0)
            return true;

        la_ssize_t written = 0;
        while (written < bytesRead) {
            const ssize_t result = ::write(fd, buffer + written, static_cast<size_t>(bytesRead - written));
            if (result < 0) {
                if (errno == EINTR)
                    continue;

                *error = errorString(errno);
                return false;
            }
            written += result;
        }

        emitProgress(static_cast<uint64_t>(std::max<la_int64_t>(0, archive_filter_bytes(reader, -1))), total);
    }
}

bool ArchiveJob::extractSingleFile(struct archive *reader, bool empty, const std::string &parentPath,
                                   uint64_t total, std::string *error)
{
    const std::string tempPath = temporaryPath(parentPath, "lingmo-extract");
    const int fd = ::open(tempPath.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0666);
    if (fd < 0) {
        *error = format(tr("Could not create {path}: {error}"),
                        {{"path", fileNameOf(m_outputPath)}, {"error", errorString(errno)}});
        return false;
    }

    currentFileChanged(fileNameOf(m_outputPath));

    const bool written = empty || extractEntryData(reader, fd, total, error);
    if (::close(fd) != 0 && written) {
        *error = errorString(errno);
        removeFile(tempPath);
        return false;
    }

    if (!written) {
        removeFile(tempPath);
        return false;
    }

    if (isCanceled() || !renameNoReplace(tempPath, m_outputPath)) {
        if (!isCanceled())
            *error = format(tr("Could not create {path}"), {{"path", m_outputPath}});
        removeFile(tempPath);
        return false;
    }

    progressChanged(100);
    return true;
}

bool ArchiveJob::extract(std::string *error)
{
    if (m_inputs.size() != 1) {
        *error = tr("Only one archive can be extracted at a time");
        return false;
    }

    std::vector<std::string> volumes;
    if (!archiveVolumes(m_inputs.front(), &volumes, error))
        return false;

    if (!isDirectory(absoluteParent(m_outputPath))) {
        *error = format(tr("The destination folder does not exist: {path}"), {{"path", absoluteParent(m_outputPath)}});
        return false;
    }

    if (extractVolumes(volumes, error))
        return true;

    if (!isCanceled() && isEncryptedRar5(volumes.front()))
        *error = tr(kEncryptedRar);
    return false;
}

bool ArchiveJob::extractVolumes(const std::vector<std::string> &volumes, std::string *error)
{
    const std::string parentPath = absoluteParent(m_outputPath);
    ArchiveReader reader = openArchiveReader(volumes, &ArchiveJob::passwordCallback, this, error);
    if (!reader)
        return false;

    // A compressed stream cannot be sized without decompressing it, so
    // progress follows how much of the archive files has been read.
    uint64_t total = 0;
    for (const std::string &volume : volumes) {
        struct stat volumeStat;
        if (::stat(volume.c_str(), &volumeStat) == 0)
            total += uint64_t(volumeStat.st_size);
    }
    progressChanged(0);

    struct archive_entry *entry = nullptr;
    int status = archive_read_next_header(reader.get(), &entry);
    if (status < ARCHIVE_WARN) {
        *error = archiveError(reader.get());
        return false;
    }

    const int format = archive_format(reader.get()) & ARCHIVE_FORMAT_BASE_MASK;
    if (format == ARCHIVE_FORMAT_RAW || format == ARCHIVE_FORMAT_EMPTY) {
        // Without a compression filter the raw format would take any file.
        if (archive_filter_count(reader.get()) < 2) {
            *error = tr("Unrecognized archive format");
            return false;
        }

        return extractSingleFile(reader.get(), status == ARCHIVE_EOF, parentPath, total, error);
    }

    const std::string tempPath = temporaryPath(parentPath, "lingmo-extract");
    if (!makePath(tempPath)) {
        *error = tr("Could not create a temporary extraction folder");
        return false;
    }

    struct Cleanup {
        std::string path;
        bool active = true;
        ~Cleanup()
        {
            if (active) {
                std::error_code ignored;
                fs::remove_all(path, ignored);
            }
        }
    } cleanup {tempPath};

    struct PendingLink {
        std::string relativePath;
        std::string target;
    };
    struct PendingDirectory {
        std::string path;
        struct timespec modified;
    };
    std::vector<PendingLink> links;
    std::vector<PendingDirectory> directories;

    for (; status != ARCHIVE_EOF; status = archive_read_next_header(reader.get(), &entry)) {
        if (status < ARCHIVE_WARN) {
            *error = archiveError(reader.get());
            return false;
        }

        if (isCanceled())
            return false;

        const std::string name = entryPathName(entry);
        std::string relativePath;
        bool directory = false;
        if (!archiveEntryPath(name, &relativePath, &directory)) {
            *error = Lingmo::i18n::format(tr("Unsafe path in archive: {path}"), {{"path", name}});
            return false;
        }

        if (archive_entry_is_encrypted(entry) && baseFormat(reader.get()) != ARCHIVE_FORMAT_ZIP) {
            *error = encryptionUnsupported(reader.get());
            return false;
        }

        if (archive_entry_hardlink(entry)) {
            *error = Lingmo::i18n::format(tr("Hard links in archives are not supported: {path}"), {{"path", name}});
            return false;
        }

        const mode_t type = archive_entry_filetype(entry);
        if (type == AE_IFLNK) {
            const std::string target = entryLinkTarget(entry);
            if (directory || relativePath.empty()) {
                *error = Lingmo::i18n::format(tr("Invalid symbolic link path in archive: {path}"), {{"path", name}});
                return false;
            }

            if (target.empty() || target.size() > 16 * 1024) {
                *error = Lingmo::i18n::format(tr("Invalid symbolic link target: {path}"), {{"path", relativePath}});
                return false;
            }

            links.push_back({relativePath, target});
            continue;
        }

        if (type != AE_IFREG && type != AE_IFDIR) {
            *error = Lingmo::i18n::format(tr("Special files in archives are not supported: {path}"), {{"path", name}});
            return false;
        }

        if (relativePath.empty())
            continue;

        const std::string destinationPath = tempPath + '/' + relativePath;
        // Set-user-ID and set-group-ID bits never survive extraction.
        const mode_t permissions = archive_entry_perm(entry) & 0777;
        currentFileChanged(relativePath);

        if (!makePath(absoluteParent(destinationPath))) {
            *error = Lingmo::i18n::format(tr("Could not create the parent folder for {path}"), {{"path", relativePath}});
            return false;
        }

        if (type == AE_IFDIR || directory) {
            // A folder first met as a parent keeps the default mode.
            if ((::mkdir(destinationPath.c_str(), permissions | S_IRWXU) != 0 && errno != EEXIST)
                || !isDirectory(destinationPath)) {
                *error = Lingmo::i18n::format(tr("Could not create {path}"), {{"path", relativePath}});
                return false;
            }

            if (archive_entry_mtime_is_set(entry))
                directories.push_back({destinationPath, entryModified(entry)});
            continue;
        }

        const int fd = ::open(destinationPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW | O_CLOEXEC,
                              permissions | S_IRUSR | S_IWUSR);
        if (fd < 0) {
            *error = Lingmo::i18n::format(tr("Could not create {path}: {error}"),
                                            {{"path", relativePath}, {"error", errorString(errno)}});
            return false;
        }

        if (!extractEntryData(reader.get(), fd, total, error)) {
            ::close(fd);
            return false;
        }

        if (archive_entry_mtime_is_set(entry)) {
            const struct timespec times[2] = {{0, UTIME_OMIT}, entryModified(entry)};
            ::futimens(fd, times);
        }

        if (::close(fd) != 0) {
            *error = Lingmo::i18n::format(tr("Could not write {path}: {error}"),
                                            {{"path", relativePath}, {"error", errorString(errno)}});
            return false;
        }
    }

    // Create symbolic links only after all regular files have been written.
    // A link inside an archive must never redirect extraction of a later file.
    for (const PendingLink &link : links) {
        if (isCanceled())
            return false;

        currentFileChanged(link.relativePath);

        const std::string destinationPath = tempPath + '/' + link.relativePath;
        if (hasSymbolicLinkParent(tempPath, link.relativePath)) {
            *error = Lingmo::i18n::format(tr("Symbolic link parent in archive: {path}"), {{"path", link.relativePath}});
            return false;
        }

        if (!makePath(absoluteParent(destinationPath))) {
            *error = Lingmo::i18n::format(tr("Could not create the parent folder for {path}"), {{"path", link.relativePath}});
            return false;
        }

        if (existsNoFollow(destinationPath)) {
            *error = Lingmo::i18n::format(tr("Duplicate path in archive: {path}"), {{"path", link.relativePath}});
            return false;
        }

        if (::symlink(link.target.c_str(), destinationPath.c_str()) != 0) {
            *error = Lingmo::i18n::format(tr("Could not restore symbolic link: {path}"), {{"path", link.relativePath}});
            return false;
        }
    }

    // Writing into a folder changes its time, so folders get theirs last.
    for (const PendingDirectory &directory : directories) {
        const struct timespec times[2] = {{0, UTIME_OMIT}, directory.modified};
        ::utimensat(AT_FDCWD, directory.path.c_str(), times, AT_SYMLINK_NOFOLLOW);
    }

    if (isCanceled() || !renameNoReplace(tempPath, m_outputPath)) {
        if (!isCanceled())
            *error = Lingmo::i18n::format(tr("Could not create {path}"), {{"path", m_outputPath}});
        return false;
    }

    cleanup.active = false;
    progressChanged(100);
    return true;
}

void ArchiveJob::zipProgressCallback(zip_t *, double progress, void *state)
{
    auto *job = static_cast<ArchiveJob *>(state);
    if (!job)
        return;

    job->emitProgress(static_cast<uint64_t>(progress * 100.0), 100);
}

int ArchiveJob::zipCancelCallback(zip_t *, void *state)
{
    auto *job = static_cast<ArchiveJob *>(state);
    return job && job->isCanceled() ? 1 : 0;
}

const char *ArchiveJob::passwordCallback(struct archive *, void *state)
{
    auto *job = static_cast<ArchiveJob *>(state);
    return job ? job->requestPassword() : nullptr;
}
