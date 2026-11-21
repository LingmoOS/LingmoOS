// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/local_file_system.h"

#include <algorithm>
#include <deque>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "base/strings.h"
#include "base/uuid.h"
#include "lingmo/files/file_enumerator.h"
#include "posix_p.h"

namespace Lingmo::Files {

using namespace internal;

// A mount is a different filesystem, or a bind of one; erasing through it would empty it.
static bool isMountRoot(int parent, const std::string& name, const struct stat& st)
{
    struct statx stx;
    if (::statx(parent, name.c_str(), AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT, 0, &stx) == 0
        && (stx.stx_attributes_mask & STATX_ATTR_MOUNT_ROOT))
        return stx.stx_attributes & STATX_ATTR_MOUNT_ROOT;

    struct stat parentSt;
    return ::fstat(parent, &parentSt) == 0 && parentSt.st_dev != st.st_dev;
}

void internal::eraseAt(int parent, const std::string& name, const std::string& path, OperationContext& ctx,
                       bool cleanup)
{
    ctx.checkpoint(path);

    struct stat st;
    if (::fstatat(parent, name.c_str(), &st, AT_SYMLINK_NOFOLLOW) < 0) {
        if (errno == ENOENT)
            return;

        throw Failure(errno, path);
    }

    if (S_ISDIR(st.st_mode)) {
        if (isMountRoot(parent, name, st))
            throw Failure(EBUSY, path);

        Fd fd(::openat(parent, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        check(fd.value, path);

        if (cleanup)
            check(::fchmod(fd.value, 0700), path);

        children(fd.value, path, [&](const std::string& child, const std::string& childPath) {
            eraseAt(fd.value, child, childPath, ctx, cleanup);
        });
    }

    ctx.checkpoint(path);
    check(::unlinkat(parent, name.c_str(), S_ISDIR(st.st_mode) ? AT_REMOVEDIR : 0), path);
    ++ctx.files;
    ctx.advance(0);
}

// Copies drive their own writeback so the page cache holds a bounded amount of unwritten data:
// progress follows the disk, and a slow drive can be ejected soon after the copy ends.
static constexpr off_t kWritebackChunk = 8 * 1024 * 1024;
static constexpr off_t kWritebackLimit = 32 * 1024 * 1024;
static constexpr size_t kWritebackFiles = 256;

namespace {

class Writeback
{
public:
    Writeback() = default;

    ~Writeback()
    {
        for (const auto& range : m_ranges)
            ::close(range.fd);
    }

    Writeback(const Writeback&) = delete;
    Writeback& operator=(const Writeback&) = delete;

    // Starts writing the range back at once and waits only for the oldest ranges, across files,
    // so the drive always has work queued and nothing forces a journal commit or cache flush.
    void push(int fd, off_t offset, off_t length, off_t bytes)
    {
        const int owned = ::fcntl(fd, F_DUPFD_CLOEXEC, 0);
        if (owned < 0)
            return;

        ::sync_file_range(owned, offset, length, SYNC_FILE_RANGE_WRITE);
        m_ranges.push_back({owned, offset, length, bytes});
        m_pending += bytes;

        while (m_pending > kWritebackLimit || m_ranges.size() > kWritebackFiles) {
            const auto range = m_ranges.front();
            m_ranges.pop_front();

            ::sync_file_range(range.fd, range.offset, range.length,
                              SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
            ::posix_fadvise(range.fd, range.offset, range.length, POSIX_FADV_DONTNEED);
            ::close(range.fd);
            m_pending -= range.bytes;
        }
    }

private:
    struct Range
    {
        int fd;
        off_t offset;
        off_t length;
        off_t bytes;
    };

    std::deque<Range> m_ranges;
    off_t m_pending = 0;
};

} // namespace

static void copyAt(int sourceParent, const std::string& source, int targetParent, const std::string& target,
                   const std::string& path, OperationContext& ctx, Writeback& writeback)
{
    ctx.checkpoint(path);

    struct stat st;
    check(::fstatat(sourceParent, source.c_str(), &st, AT_SYMLINK_NOFOLLOW), path);

    if (S_ISLNK(st.st_mode)) {
        std::string link(256, '\0');

        for (;;) {
            const ssize_t length = ::readlinkat(sourceParent, source.c_str(), link.data(), link.size());
            check(length, path);

            if (size_t(length) < link.size()) {
                link.resize(size_t(length));
                break;
            }

            link.resize(link.size() * 2);
        }

        check(::symlinkat(link.c_str(), targetParent, target.c_str()), path);
    } else if (S_ISDIR(st.st_mode)) {
        Fd input(::openat(sourceParent, source.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        check(input.value, path);
        check(::mkdirat(targetParent, target.c_str(), 0700), path);

        Fd output(::openat(targetParent, target.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        check(output.value, path);

        children(input.value, path, [&](const std::string& child, const std::string& childPath) {
            copyAt(input.value, child, output.value, child, childPath, ctx, writeback);
        });

        check(::fchmod(output.value, st.st_mode & 0777), path);
        const timespec times[] = {st.st_atim, st.st_mtim};
        check(::futimens(output.value, times), path);
    } else if (S_ISREG(st.st_mode)) {
        Fd input(::openat(sourceParent, source.c_str(), O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC));
        check(input.value, path);

        struct stat opened;
        check(::fstat(input.value, &opened), path);
        if (!S_ISREG(opened.st_mode) || opened.st_ino != st.st_ino || opened.st_dev != st.st_dev)
            throw Failure(ESTALE, path);

        Fd output(::openat(targetParent, target.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
        check(output.value, path);

        std::string buffer;
        off_t position = 0;
        bool sparse = true;
        bool inKernel = true;

        off_t flushed = 0;
        off_t unflushed = 0;
        auto wrote = [&](off_t bytes) {
            unflushed += bytes;
            if (unflushed < kWritebackChunk)
                return;

            writeback.push(output.value, flushed, position - flushed, unflushed);
            flushed = position;
            unflushed = 0;
        };

        // Filesystems that share extents (Btrfs, XFS) clone the whole file; the rest refuse.
        if (opened.st_size > 0 && ::ioctl(output.value, FICLONE, input.value) == 0) {
            ctx.advance(uint64_t(opened.st_size));
            position = opened.st_size;
        }

        while (position < opened.st_size) {
            ctx.checkpoint(path);
            off_t extentEnd = opened.st_size;

            if (sparse) {
                const off_t data = ::lseek(input.value, position, SEEK_DATA);
                if (data < 0 && errno == ENXIO) {
                    ctx.advance(uint64_t(opened.st_size - position));
                    position = opened.st_size;
                    break;
                }

                if (data < 0 && (errno == EINVAL || errno == ENOTSUP)) {
                    sparse = false;
                } else {
                    if (data < 0)
                        throw Failure(errno, path);

                    ctx.advance(uint64_t(data - position));
                    position = data;

                    extentEnd = ::lseek(input.value, position, SEEK_HOLE);
                    if (extentEnd < 0)
                        throw Failure(errno, path);

                    extentEnd = std::min(extentEnd, opened.st_size);
                }
            }

            while (position < extentEnd) {
                ctx.checkpoint(path);

                if (inKernel) {
                    off_t from = position;
                    off_t to = position;
                    const size_t length = size_t(std::min<off_t>(4 * 1024 * 1024, extentEnd - position));
                    const ssize_t copied = ::copy_file_range(input.value, &from, output.value, &to, length, 0);

                    if (copied < 0 && errno == EINTR)
                        continue;

                    if (copied > 0) {
                        ctx.advance(uint64_t(copied));
                        position += copied;
                        wrote(copied);
                        continue;
                    }

                    // Refused (another filesystem, no support) or cut short: the buffered copy
                    // takes over and reports a genuine failure itself.
                    inKernel = false;
                }

                if (buffer.empty())
                    buffer.resize(1024 * 1024);

                const ssize_t count = ::pread(input.value, buffer.data(),
                                              size_t(std::min<off_t>(off_t(buffer.size()), extentEnd - position)),
                                              position);
                if (count < 0 && errno == EINTR)
                    continue;

                check(count, path);
                if (!count)
                    throw Failure(ESTALE, path);

                ssize_t offset = 0;
                while (offset < count) {
                    ctx.checkpoint(path);

                    const ssize_t written =
                        ::pwrite(output.value, buffer.data() + offset, size_t(count - offset), position + offset);
                    if (written < 0 && errno == EINTR)
                        continue;

                    check(written, path);
                    if (!written)
                        throw Failure(EIO, path);

                    offset += written;
                    ctx.advance(uint64_t(written));
                }

                position += count;
                wrote(count);
            }
        }

        if (unflushed)
            writeback.push(output.value, flushed, position - flushed, unflushed);

        check(::ftruncate(output.value, opened.st_size), path);
        check(::fchmod(output.value, st.st_mode & 0777), path);
        const timespec times[] = {st.st_atim, st.st_mtim};
        check(::futimens(output.value, times), path);
    } else {
        throw Failure(ENOTSUP, path);
    }

    ++ctx.files;
}

static bool isSymbolicLink(const std::string& path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0 && S_ISLNK(st.st_mode);
}

static void validateDestination(const std::string& source, const std::string& destination)
{
    const std::string src = canonicalPath(source);
    const std::string dstParent = canonicalPath(absolutePathOf(destination));
    if (!src.empty() && !isSymbolicLink(source) && (dstParent == src || dstParent.starts_with(src + '/')))
        throw Failure(EINVAL, destination);

    struct stat a, b;
    if (::lstat(native(source).c_str(), &a) == 0 && ::lstat(native(destination).c_str(), &b) == 0
        && a.st_dev == b.st_dev && a.st_ino == b.st_ino)
        throw Failure(EINVAL, destination);
}

// Neither the source nor a folder that holds it can be replaced: a link would take
// the place of what it points to, and merging into the folder would write the source's
// copy into the source itself, which a move would then delete.
static void validateReplacement(const std::string& source, const std::string& destination)
{
    struct stat a, b;
    if (::lstat(native(source).c_str(), &a) == 0 && ::lstat(native(destination).c_str(), &b) == 0
        && a.st_dev == b.st_dev && a.st_ino == b.st_ino)
        throw Failure(EINVAL, destination);

    const std::string sourceParent = canonicalPath(absolutePathOf(source));
    const std::string destinationParent = canonicalPath(absolutePathOf(destination));
    if (sourceParent.empty() || destinationParent.empty())
        return;

    const std::string src = joined(sourceParent, fileNameOf(source));
    const std::string dst = joined(destinationParent, fileNameOf(destination));
    if (src.starts_with(dst + '/'))
        throw Failure(EINVAL, destination);
}

static void mergeStaged(int source, int target, const std::string& path, OperationContext& ctx)
{
    children(source, path, [&](const std::string& name, const std::string& childPath) {
        ctx.checkpoint(childPath);

        struct stat from, to;
        check(::fstatat(source, name.c_str(), &from, AT_SYMLINK_NOFOLLOW), childPath);
        const bool exists = ::fstatat(target, name.c_str(), &to, AT_SYMLINK_NOFOLLOW) == 0;

        if (S_ISDIR(from.st_mode) && exists && S_ISDIR(to.st_mode)) {
            Fd input(::openat(source, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
            Fd output(::openat(target, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
            check(input.value, childPath);
            check(output.value, childPath);
            check(::fchmod(input.value, 0700), childPath);
            mergeStaged(input.value, output.value, childPath, ctx);
            check(::unlinkat(source, name.c_str(), AT_REMOVEDIR), childPath);
        } else {
            check(::renameat(source, name.c_str(), target, name.c_str()), childPath);
        }
    });
}

static bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

// "name (n).ext", the name an item kept beside an existing one takes.
static std::string keptName(const std::string& destination, int suffix)
{
    // The extension follows the name's last dot, even a leading one; the base
    // name is everything before that dot.
    const std::string name = fileNameOf(destination);
    const size_t dot = name.rfind('.');
    const bool hasExtension = !isDirectory(destination) && dot != std::string::npos && dot + 1 < name.size();
    const std::string extension = hasExtension ? name.substr(dot) : std::string();
    const std::string base = hasExtension ? name.substr(0, dot) : name;
    return native(base + " (" + base::strings::number(suffix) + ")" + extension);
}

static std::string temporaryName(const char* prefix)
{
    std::string id = base::Uuid::generate().toString();
    std::erase(id, '-');
    return std::string(prefix) + id;
}

static std::string publish(int parent, const std::string& temporary, const std::string& source,
                           const std::string& destination, ConflictPolicy policy, OperationContext& ctx)
{
    std::string name = native(fileNameOf(destination));
    int suffix = 1;

    for (;;) {
        if (::syscall(SYS_renameat2, parent, temporary.c_str(), parent, name.c_str(), RENAME_NOREPLACE) == 0)
            return absolutePathOf(destination) + '/' + name;

        if (errno != EEXIST)
            throw Failure(errno, destination);

        if (policy == ConflictPolicy::Ask) {
            policy = ctx.conflict ? ctx.conflict(source, destination) : ConflictPolicy::Fail;
            ctx.checkpoint(destination);
        }

        if (policy == ConflictPolicy::Skip)
            return {};

        if (policy == ConflictPolicy::Fail)
            throw Failure(EEXIST, destination);

        if (policy == ConflictPolicy::Replace) {
            validateReplacement(source, destination);

            // Copies are not synced one by one; the replacement must be on disk before the
            // original goes.
            check(::syncfs(parent), destination);

            struct stat from, to;
            check(::fstatat(parent, temporary.c_str(), &from, AT_SYMLINK_NOFOLLOW), destination);
            check(::fstatat(parent, name.c_str(), &to, AT_SYMLINK_NOFOLLOW), destination);

            if (S_ISDIR(from.st_mode) && S_ISDIR(to.st_mode)) {
                Fd input(::openat(parent, temporary.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
                Fd output(::openat(parent, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
                check(input.value, destination);
                check(output.value, destination);
                check(::fchmod(input.value, 0700), destination);
                mergeStaged(input.value, output.value, destination, ctx);
                check(::unlinkat(parent, temporary.c_str(), AT_REMOVEDIR), destination);
                ctx.merged = true;
            } else {
                check(::renameat(parent, temporary.c_str(), parent, name.c_str()), destination);
            }

            return destination;
        }

        name = keptName(destination, suffix++);
    }
}

std::string LocalFileSystem::copy(const std::string& source, const std::string& destination, ConflictPolicy policy,
                                  OperationContext& ctx)
{
    // Pasting an item back into its own folder always means "make a copy":
    // there is nothing to replace, so never ask.
    const bool duplicate = cleanPath(source) == cleanPath(destination);
    if (duplicate && policy == ConflictPolicy::Ask)
        policy = ConflictPolicy::KeepBoth;
    if (!duplicate || policy != ConflictPolicy::KeepBoth)
        validateDestination(source, destination);

    ctx.checkpoint(source);
    Parent input(source), output(destination);
    const std::string temporary = temporaryName(".lingmo-copy-");

    try {
        Writeback writeback;
        copyAt(input.fd.value, input.name, output.fd.value, temporary, source, ctx, writeback);
        ctx.checkpoint(source);

        auto result = publish(output.fd.value, temporary, source, destination, policy, ctx);
        if (!result.empty())
            return result;
    } catch (...) {
        OperationContext cleanup;
        try {
            eraseAt(output.fd.value, temporary, destination, cleanup, true);
        } catch (...) {
        }
        throw;
    }

    OperationContext cleanup;
    eraseAt(output.fd.value, temporary, destination, cleanup, true);
    return {};
}

std::string LocalFileSystem::link(const std::string& source, const std::string& destination, ConflictPolicy policy,
                                  OperationContext& ctx)
{
    ctx.checkpoint(source);
    Parent output(destination);
    const std::string temporary = temporaryName(".lingmo-link-");
    check(::symlinkat(native(source).c_str(), output.fd.value, temporary.c_str()), destination);

    try {
        const auto result = publish(output.fd.value, temporary, source, destination, policy, ctx);
        if (result.empty())
            ::unlinkat(output.fd.value, temporary.c_str(), 0);

        return result;
    } catch (...) {
        ::unlinkat(output.fd.value, temporary.c_str(), 0);
        throw;
    }
}

// Where names ignore case, the new case of a name finds the item itself. A hard link
// in the same folder is the same inode too, but only under another name.
static bool isNewCase(const std::string& source, const std::string& destination)
{
    const std::string sourceParent = canonicalPath(absolutePathOf(source));
    if (fileNameOf(source) == fileNameOf(destination) || sourceParent.empty()
        || sourceParent != canonicalPath(absolutePathOf(destination)))
        return false;

    struct stat a, b;
    return ::lstat(native(source).c_str(), &a) == 0 && ::lstat(native(destination).c_str(), &b) == 0
        && a.st_dev == b.st_dev && a.st_ino == b.st_ino && (S_ISDIR(a.st_mode) || a.st_nlink == 1);
}

std::string LocalFileSystem::move(const std::string& source, const std::string& destination, ConflictPolicy policy,
                                  OperationContext& ctx)
{
    if (isNewCase(source, destination)) {
        ctx.checkpoint(source);
        Parent input(source), output(destination);

        // Renaming an item to itself does nothing, so it goes by way of another name.
        const std::string temporary = temporaryName(".lingmo-rename-");
        if (::syscall(SYS_renameat2, input.fd.value, input.name.c_str(), input.fd.value, temporary.c_str(),
                      RENAME_NOREPLACE)
            != 0)
            throw Failure(errno, source);

        if (::syscall(SYS_renameat2, input.fd.value, temporary.c_str(), output.fd.value, output.name.c_str(),
                      RENAME_NOREPLACE)
            != 0) {
            const int code = errno;
            ::renameat(input.fd.value, temporary.c_str(), input.fd.value, input.name.c_str());
            throw Failure(code, destination);
        }

        return destination;
    }

    validateDestination(source, destination);
    ctx.checkpoint(source);
    Parent input(source), output(destination);

    if (::syscall(SYS_renameat2, input.fd.value, input.name.c_str(), output.fd.value, output.name.c_str(),
                  RENAME_NOREPLACE)
        == 0)
        return destination;

    const int code = errno;
    if (code == EEXIST && policy == ConflictPolicy::Ask) {
        policy = ctx.conflict ? ctx.conflict(source, destination) : ConflictPolicy::Fail;
        ctx.checkpoint(destination);
    }

    if (code == EEXIST && policy == ConflictPolicy::Skip)
        return {};

    if (code == EEXIST && policy == ConflictPolicy::Fail)
        throw Failure(code, destination);

    if (code == EEXIST && policy == ConflictPolicy::KeepBoth) {
        for (int suffix = 1;; ++suffix) {
            const std::string name = keptName(destination, suffix);
            if (::syscall(SYS_renameat2, input.fd.value, input.name.c_str(), output.fd.value, name.c_str(),
                          RENAME_NOREPLACE)
                == 0)
                return absolutePathOf(destination) + '/' + name;
            if (errno != EEXIST)
                break;
        }
    }

    if (code == EEXIST && policy == ConflictPolicy::Replace) {
        validateReplacement(source, destination);
        if (::renameat(input.fd.value, input.name.c_str(), output.fd.value, output.name.c_str()) == 0)
            return destination;
        if (errno != EXDEV && errno != ENOTEMPTY && errno != EEXIST)
            throw Failure(errno, destination);
    } else if (code != EXDEV && code != EEXIST) {
        throw Failure(code, destination);
    }

    auto result = copy(source, destination, policy, ctx);

    // Once a cross-device copy is published, finish removal without cancellation.
    if (!result.empty()) {
        // The source is the only durable copy until the target reaches the disk.
        check(::syncfs(output.fd.value), result);
        OperationContext removal;
        remove(source, removal);
    }

    return result;
}

void LocalFileSystem::remove(const std::string& path, OperationContext& ctx)
{
    Parent parent(path);
    eraseAt(parent.fd.value, parent.name, path, ctx);
}

void LocalFileSystem::mkdir(const std::string& path)
{
    check(::mkdir(native(path).c_str(), 0777), path);
}

FileItemList LocalFileSystem::enumerate(const Url& url, bool hidden)
{
    return FileEnumerator::list(url, hidden);
}

uint64_t LocalFileSystem::directorySize(const std::string& path, OperationContext& ctx)
{
    return FileEnumerator::size(path, ctx);
}

} // namespace Lingmo::Files
