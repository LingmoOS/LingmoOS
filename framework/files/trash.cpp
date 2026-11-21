// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <ctime>
#include <memory>
#include <optional>
#include <sys/file.h>
#include <sys/stat.h>

#include "base/file.h"
#include "base/strings.h"
#include "base/url.h"
#include "base/uuid.h"
#include "lingmo/files/local_file_system.h"
#include "framework_i18n.h"
#include "posix_p.h"
#include "trash_directory_p.h"

namespace Lingmo::Files {

using namespace internal;
using Lingmo::framework::tr;

static void ensureHomeTrash()
{
    const auto root = FileSystemUrl::trashRoot();

    for (const auto& path : {root, root + "/files", root + "/info"}) {
        if (!makePath(path))
            throw Failure(EACCES, path);

        struct stat st;
        check(::lstat(native(path).c_str(), &st), path);

        if (!S_ISDIR(st.st_mode) || st.st_uid != ::getuid())
            throw Failure(EACCES, path);

        check(::chmod(native(path).c_str(), 0700), path);
    }
}

// The home trash may not exist yet; the nearest folder that does stands in for it.
static dev_t homeTrashDevice()
{
    std::string path = FileSystemUrl::trashRoot();
    struct stat st;

    while (::stat(native(path).c_str(), &st) < 0) {
        const std::string parent = absolutePathOf(path);
        if (parent == path)
            throw Failure(errno, path);

        path = parent;
    }

    return st.st_dev;
}

// The mount point of the filesystem path is on, path being canonical.
static std::string mountPointOf(std::string path)
{
    while (path != "/") {
        struct statx stx;
        if (::statx(AT_FDCWD, native(path).c_str(), AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT, 0, &stx) == 0
            && (stx.stx_attributes_mask & STATX_ATTR_MOUNT_ROOT)) {
            if (stx.stx_attributes & STATX_ATTR_MOUNT_ROOT)
                return path;
        } else {
            // Kernels before 5.8 do not say; the device changes at a mount.
            struct stat st, parentSt;
            if (::stat(native(path).c_str(), &st) == 0 && ::stat(absolutePathOf(path).c_str(), &parentSt) == 0
                && st.st_dev != parentSt.st_dev)
                return path;
        }

        path = absolutePathOf(path);
    }

    return path;
}

// Files on the home trash's filesystem go there. Others go to their own volume's
// trash, so trashing frees no space but costs no copy; only a volume that cannot
// take a trash (remote, read-only, no permission) sends them home.
static TrashDirectory trashFor(const std::string& source)
{
    const std::string parent = canonicalPath(absolutePathOf(source));
    const dev_t home = homeTrashDevice();

    struct stat st;
    if (!parent.empty() && ::stat(native(parent).c_str(), &st) == 0 && st.st_dev != home) {
        const std::string topdir = mountPointOf(parent);

        if (volumeTakesTrash(topdir) && ::stat(native(topdir).c_str(), &st) == 0 && st.st_dev != home) {
            for (const bool shared : {true, false}) {
                const auto trash = volumeTrash(topdir, shared);
                if (prepareVolumeTrash(trash))
                    return trash;
            }
        }
    }

    ensureHomeTrash();
    return homeTrash();
}

static bool isInside(const std::string& path, const std::string& directory)
{
    const std::string clean = cleanPath(path);
    return clean == directory || clean.starts_with(directory + '/');
}

static bool existsOrLinks(const std::string& path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0;
}

namespace {

class TrashLock
{
public:
    TrashLock(const TrashDirectory& trash, OperationContext& ctx)
        : m_fd(::open(native(trash.path + "/.lock").c_str(), O_CREAT | O_RDWR | O_NOFOLLOW | O_CLOEXEC, 0600))
    {
        check(m_fd.value, trash.path);

        while (::flock(m_fd.value, LOCK_EX | LOCK_NB) < 0) {
            if (errno != EWOULDBLOCK && errno != EINTR)
                throw Failure(errno, trash.path);

            ctx.checkpoint();
            timespec delay{0, 25000000};
            ::nanosleep(&delay, nullptr);
        }

        ctx.checkpoint();
    }

private:
    Fd m_fd;
};

} // namespace

// The trash of an item at the top of one, and its name there.
static TrashDirectory trashedItem(const Url& url, std::string* name)
{
    FileSystemUrl validated(url);
    const auto trash = trashDirectory(url, name);
    if (name->empty() || name->find('/') != std::string::npos)
        throw Failure(EINVAL, url.toString());

    return trash;
}

static std::string infoPath(const TrashDirectory& trash, const std::string& name)
{
    return trash.path + "/info/" + name + ".trashinfo";
}

// The trash specification's cache of trashed folders' sizes, which other
// implementations write: a line of size, info file time and encoded name each.
static std::string directorySizesPath(const TrashDirectory& trash)
{
    return trash.path + "/directorysizes";
}

// A size cached for an item no longer in the trash would be read for the next item
// of that name.
static void forgetDirectorySize(const TrashDirectory& trash, const std::string& name)
{
    const auto path = directorySizesPath(trash);
    const std::optional<std::string> contents = readTrashFile(path);
    if (!contents)
        return;

    std::string kept;
    bool forgotten = false;

    for (const std::string_view line : base::strings::splitViews(*contents, '\n')) {
        const size_t size = line.find(' ');
        const size_t time = size == std::string_view::npos ? size : line.find(' ', size + 1);
        if (time != std::string_view::npos && base::url::percentDecoded(line.substr(time + 1)) == name) {
            forgotten = true;
            continue;
        }

        if (!line.empty()) {
            kept += line;
            kept += '\n';
        }
    }

    if (forgotten)
        base::file::writeAtomically(path, kept);
}

void LocalFileSystem::remove(const Url& url, OperationContext& ctx)
{
    const auto path = FileSystemUrl(url).localPath();

    if (url.isLocalFile()) {
        remove(path, ctx);
        return;
    }

    std::string inside;
    const auto trash = trashDirectory(url, &inside);
    if (inside.empty())
        throw Failure(EINVAL, url.toString());

    if (trash.topdir.empty())
        ensureHomeTrash();

    TrashLock lock(trash, ctx);
    remove(path, ctx);

    if (inside.find('/') == std::string::npos) {
        const auto info = infoPath(trash, inside);
        if (::unlink(native(info).c_str()) < 0 && errno != ENOENT)
            throw Failure(errno, info);

        forgetDirectorySize(trash, inside);
    }
}

// Drops the last character, never part of one.
static void chopCharacter(std::string& text)
{
    while (!text.empty() && (static_cast<unsigned char>(text.back()) & 0xC0) == 0x80)
        text.pop_back();
    if (!text.empty())
        text.pop_back();
}

static std::string uuidHex()
{
    std::string id = base::Uuid::generate().toString();
    std::erase(id, '-');
    return id;
}

Url LocalFileSystem::trash(const Url& url, OperationContext& ctx)
{
    if (!url.isLocalFile())
        throw Failure(EINVAL, url.toString());

    const auto source = FileSystemUrl(url).localPath();
    if (isInside(source, FileSystemUrl::trashRoot()))
        throw Failure(EINVAL, source);

    // Neither trash of a volume takes the other, or itself.
    const auto trash = trashFor(source);
    const std::string canonical = canonicalPath(absolutePathOf(source)) + '/' + url.fileName();
    if (!trash.topdir.empty()
        && (isInside(canonical, volumeTrash(trash.topdir, true).path)
            || isInside(canonical, volumeTrash(trash.topdir, false).path)))
        throw Failure(EINVAL, source);

    TrashLock lock(trash, ctx);

    std::string name = url.fileName();
    std::string info;
    std::unique_ptr<Fd> infoFile;

    for (int i = 0;; ++i) {
        std::string base = url.fileName();
        while (base.size() > 180)
            chopCharacter(base);

        name = base + (i ? '-' + uuidHex() : std::string());
        if (existsOrLinks(trash.path + "/files/" + name))
            continue;

        info = infoPath(trash, name);
        infoFile = std::make_unique<Fd>(
            ::open(native(info).c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
        if (infoFile->value >= 0)
            break;

        if (errno != EEXIST)
            throw Failure(errno, info);
    }

    // A volume's trash records paths from its mount point, which can differ next time.
    std::string original;
    if (trash.topdir.empty()) {
        original = absoluteFilePath(source);
    } else {
        const std::string top = cleanPath(trash.topdir);
        original = top == "/" ? canonical.substr(1)
                              : canonical.starts_with(top + '/') ? canonical.substr(top.size() + 1) : canonical;
    }

    char date[32];
    const time_t now = ::time(nullptr);
    struct tm local;
    ::localtime_r(&now, &local);
    ::strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", &local);

    const std::string content =
        "[Trash Info]\nPath=" + base::url::percentEncoded(original, "/") + "\nDeletionDate=" + date + '\n';

    try {
        errno = 0;
        if (::write(infoFile->value, content.data(), content.size()) != ssize_t(content.size()))
            throw Failure(errno ? errno : EIO, info);

        check(::fsync(infoFile->value), info);
        move(source, trash.path + "/files/" + name, ConflictPolicy::Fail, ctx);
    } catch (...) {
        if (!existsOrLinks(trash.path + "/files/" + name))
            ::unlink(native(info).c_str());
        throw;
    }

    Url result("trash:/");
    result.setPath('/' + trash.id + '/' + name);
    return result;
}

// A link on a volume can lead anywhere: the folder an item goes back to, as far as
// it exists, resolves inside the volume.
static bool staysOnVolume(const TrashDirectory& trash, std::string folder)
{
    struct stat st;
    while (folder != "/" && ::lstat(native(folder).c_str(), &st) != 0 && errno == ENOENT)
        folder = absolutePathOf(folder);

    const std::string real = canonicalPath(folder);
    const std::string top = canonicalPath(trash.topdir);
    return !real.empty() && !top.empty() && (top == "/" || isInside(real, top));
}

Url LocalFileSystem::restore(const Url& url, ConflictPolicy policy, OperationContext& ctx)
{
    std::string name;
    const auto trash = trashedItem(url, &name);
    if (trash.topdir.empty())
        ensureHomeTrash();

    TrashLock lock(trash, ctx);

    const auto info = infoPath(trash, name);
    TrashInfo details;
    if (!readTrashInfo(trash, name, &details))
        throw Failure(existsOrLinks(info) ? EACCES : ENOENT, info);

    if (details.path.empty())
        throw Failure(EINVAL, name, tr("The original location is not known"));

    // A folder deleted since the item was trashed is made again.
    const std::string parent = absolutePathOf(details.path);
    if (!trash.topdir.empty() && !staysOnVolume(trash, parent))
        throw Failure(EXDEV, name, tr("The original location is outside the volume it was deleted on"));

    errno = 0;
    if (!makePath(parent))
        throw Failure(errno ? errno : EIO, parent);

    auto result = move(FileSystemUrl(url).localPath(), details.path, policy, ctx);
    if (result.empty())
        return {};

    check(::unlink(native(info).c_str()), info);
    forgetDirectorySize(trash, name);
    return Url::fromLocalFile(result);
}

// Empties one trash, each item before its info file, so a cancelled run leaves no
// item without one.
static void empty(const TrashDirectory& trash, OperationContext& ctx)
{
    struct stat st;
    if (trash.topdir.empty())
        ensureHomeTrash();
    else if (::stat(trash.path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
        return;

    TrashLock lock(trash, ctx);

    const auto erase = [&](const std::string& directory, auto visit) {
        Fd fd(::open(native(directory).c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        if (fd.value < 0 && errno == ENOENT)
            return;

        check(fd.value, directory);
        children(fd.value, directory, [&](const std::string& name, const std::string& path) {
            visit(fd.value, name, path);
        });
    };

    erase(trash.path + "/files", [&](int fd, const std::string& name, const std::string& path) {
        eraseAt(fd, name, path, ctx);

        const auto info = infoPath(trash, name);
        if (::unlink(native(info).c_str()) < 0 && errno != ENOENT)
            throw Failure(errno, info);
    });

    erase(trash.path + "/info", [&](int fd, const std::string& name, const std::string& path) {
        if (name.ends_with(".trashinfo"))
            eraseAt(fd, name, path, ctx);
    });

    const auto sizes = directorySizesPath(trash);
    if (::unlink(native(sizes).c_str()) < 0 && errno != ENOENT)
        throw Failure(errno, sizes);
}

// A volume that fails to empty, being read-only say, does not keep the others full.
void LocalFileSystem::emptyTrash(OperationContext& ctx)
{
    std::optional<Failure> failure;

    for (const auto& trash : trashDirectories()) {
        try {
            empty(trash, ctx);
        } catch (const Failure& error) {
            if (error.error.code == ECANCELED)
                throw;

            if (!failure)
                failure = error;
        }
    }

    if (failure)
        throw *failure;
}

} // namespace Lingmo::Files
