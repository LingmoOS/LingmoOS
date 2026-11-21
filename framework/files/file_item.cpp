// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/file_item.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/locale.h"
#include "base/paths.h"
#include "lingmo/files/error.h"
#include "lingmo/files/file_system_url.h"
#include "lingmo/files/mime_types.h"
#include "mime_sniffer_p.h"
#include "path_utils_p.h"
#include "trash_directory_p.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;

static const std::string s_empty;
static const Url s_emptyUrl;

static int64_t milliseconds(const statx_timestamp& time)
{
    return int64_t(time.tv_sec) * 1000 + time.tv_nsec / 1000000;
}

FileItem::FileItem(const Url& url)
{
    try {
        const std::string path = FileSystemUrl(url).localPath();
        d = std::make_shared<Data>();
        d->url = url;
        d->name = url.fileName();
        d->localPath = path;
        readStat(&d->stat);

        std::string name;
        internal::TrashInfo info;
        if (url.scheme() == "trash" && url.path() != "/") {
            const auto trash = internal::trashDirectory(url, &name);
            if (!name.empty() && name.find('/') == std::string::npos && internal::readTrashInfo(trash, name, &info)) {
                d->originalPath = info.path;
                d->deletionTime = info.deletionTime;
            }
        }
    } catch (const Failure&) {
        d.reset();
    }
}

FileItem::FileItem(const Url& url, std::string localPath, std::string name)
    : d(std::make_shared<Data>())
{
    d->url = url;
    d->name = std::move(name);
    d->localPath = std::move(localPath);
}

FileItem::Data& FileItem::detach()
{
    if (!d)
        d = std::make_shared<Data>();
    else if (d.use_count() > 1)
        d = std::make_shared<Data>(*d);
    return *d;
}

const Url& FileItem::url() const
{
    return d ? d->url : s_emptyUrl;
}

const std::string& FileItem::localPath() const
{
    return d ? d->localPath : s_empty;
}

const std::string& FileItem::name() const
{
    return d ? d->name : s_empty;
}

std::string FileItem::text() const
{
    return !d || d->displayName.empty() ? name() : d->displayName;
}

void FileItem::setDisplayName(const std::string& name)
{
    detach().displayName = name;
}

void FileItem::setIconName(const std::string& icon)
{
    detach().iconName = icon;
}

Url FileItem::targetUrl() const
{
    return d ? d->targetUrl : Url();
}

void FileItem::setTargetUrl(const Url& url)
{
    detach().targetUrl = url;
}

// The link and its target's stat, and write access; no accessor uses read or execute access.
void FileItem::readStat(Stat* result) const
{
    *result = {};
    result->read = true;

    const std::string& path = d->localPath;
    struct statx own;
    if (path.empty()
        || ::statx(AT_FDCWD, path.c_str(), AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT,
                   STATX_BASIC_STATS | STATX_BTIME, &own) != 0)
        return;

    result->link = S_ISLNK(own.stx_mode);

    struct statx target;
    const struct statx* used = &own;
    if (result->link) {
        if (::statx(AT_FDCWD, path.c_str(), AT_NO_AUTOMOUNT, STATX_BASIC_STATS | STATX_BTIME, &target) == 0)
            used = &target;
        else
            used = nullptr;
    }

    const struct statx& info = used ? *used : own;
    result->exists = used != nullptr;
    result->file = used && S_ISREG(info.stx_mode);
    result->dir = used && S_ISDIR(info.stx_mode);
    if (used && !result->file && !result->dir)
        result->special = info.stx_mode & S_IFMT;
    result->size = info.stx_size;
    if (info.stx_mask & STATX_BTIME)
        result->birth = milliseconds(info.stx_btime);
    result->access = milliseconds(info.stx_atime);
    result->modified = milliseconds(info.stx_mtime);
    result->changed = milliseconds(info.stx_ctime);
    result->writable = result->exists && ::access(path.c_str(), W_OK) == 0;
}

const FileItem::Stat& FileItem::stat() const
{
    static const Stat none;
    if (!d)
        return none;

    if (!d->stat.read)
        readStat(&d->stat);
    return d->stat;
}

bool FileItem::exists() const
{
    return stat().exists;
}

bool FileItem::isFile() const
{
    return stat().file;
}

bool FileItem::isDir() const
{
    return stat().dir;
}

bool FileItem::isLink() const
{
    return stat().link;
}

bool FileItem::isHidden() const
{
    return d && internal::fileNameOf(d->localPath).starts_with('.');
}

bool FileItem::isWritable() const
{
    return stat().writable;
}

bool FileItem::isDesktopFile() const
{
    return name().ends_with(".desktop");
}

uint64_t FileItem::size() const
{
    return stat().size;
}

std::string FileItem::originalPath() const
{
    return d ? d->originalPath : std::string();
}

std::optional<int64_t> FileItem::deletionTime() const
{
    return d ? d->deletionTime : std::nullopt;
}

std::optional<int64_t> FileItem::time(FileTime type) const
{
    const Stat& info = stat();
    if (!info.read || (!info.exists && !info.link))
        return std::nullopt;

    switch (type) {
    case CreationTime:
        return info.birth;
    case AccessTime:
        return info.access;
    case ModificationTime:
        break;
    }

    return info.modified;
}

void FileItem::prepareMetadata()
{
    prepareMetadata(FileItem(), nullptr);
}

bool FileItem::prepareMetadata(const FileItem& previous, TypeDetails* details)
{
    if (isNull())
        return false;

    Data& data = detach();
    readStat(&data.stat);

    data.stamp = {};
    if (data.stat.dir || data.stat.file) {
        data.stamp.size = int64_t(data.stat.size);
        data.stamp.modified = data.stat.modified;
        data.stamp.changed = data.stat.changed;
        data.stamp.kind = (data.stat.dir ? 1 : 2) | (data.stat.link ? 4 : 0);
    }

    // Content sniffing reads the file; the same entry with the same size and times
    // has the same contents.
    if (previous.d && data.stamp.kind && data.stamp == previous.d->stamp && data.url == previous.d->url) {
        data.mimeType = previous.d->mimeType;
        data.typeDescription = previous.d->typeDescription;
        data.descriptionLocale = previous.d->descriptionLocale;
        data.iconName = previous.d->iconName;
        data.displayName = previous.d->displayName;
        data.targetUrl = previous.d->targetUrl;
        return true;
    }

    detectType(details);
    return false;
}

void FileItem::detectType(TypeDetails* details)
{
    Data& data = *d;

    if (data.stat.dir) {
        data.mimeType = "inode/directory";
    } else if (data.stat.file) {
        // mime::typeForFile() would stat the file again to rule out devices.
        data.mimeType = internal::MimeSniffer::self()->mimeTypeForFile(data.localPath);
    } else if (data.stat.special == S_IFCHR) {
        data.mimeType = "inode/chardevice";
    } else if (data.stat.special == S_IFBLK) {
        data.mimeType = "inode/blockdevice";
    } else if (data.stat.special == S_IFIFO) {
        data.mimeType = "inode/fifo";
    } else if (data.stat.special == S_IFSOCK) {
        data.mimeType = "inode/socket";
    } else {
        data.mimeType = internal::MimeSniffer::self()->mimeTypeForFile(data.localPath);
    }

    // A type's description and icon cost a database lookup each, and the
    // description a walk over the UI languages.
    std::string typeIcon;
    const auto known = details ? details->find(data.mimeType) : TypeDetails::iterator();

    data.descriptionLocale = &Locale::system();
    if (details && known != details->end()) {
        data.typeDescription = known->second.first;
        typeIcon = known->second.second;
    } else {
        data.typeDescription = mime::description(data.mimeType);
        typeIcon = mime::iconName(data.mimeType);

        if (details)
            details->emplace(data.mimeType, std::make_pair(data.typeDescription, typeIcon));
    }

    if (data.iconName.empty())
        data.iconName = data.stat.dir ? iconName() : typeIcon;
}

std::string FileItem::mimetype() const
{
    if (isNull())
        return {};

    if (!d->mimeType.empty())
        return d->mimeType;

    return mime::typeForFile(d->localPath);
}

std::string FileItem::typeDescription() const
{
    if (!d)
        return {};
    // Read again in the new language after the session's changed.
    if (d->descriptionLocale != &Locale::system())
        return mime::description(d->mimeType);
    return d->typeDescription;
}

std::string FileItem::iconName() const
{
    if (d && !d->iconName.empty())
        return d->iconName;

    if (!isDir())
        return isNull() ? std::string() : mime::iconName(mimetype());

    const std::string standard = standardFolderIconName(localPath());
    return standard.empty() ? std::string("folder") : standard;
}

bool FileItem::sameStat(const FileItem& other) const
{
    return d && other.d && d->stamp.kind && d->stamp == other.d->stamp;
}

bool FileItemListProperties::supportsMoving() const
{
    if (m_items.empty())
        return false;

    for (const auto& item : m_items) {
        if (item.isNull() || ::access(internal::absolutePathOf(item.localPath()).c_str(), W_OK) != 0)
            return false;
    }

    return true;
}

std::string convertSize(uint64_t size)
{
    return Locale::system().formattedDataSize(int64_t(size));
}

std::string standardFolderIconName(const std::string& path)
{
    if (path.empty())
        return {};

    // user-dirs.dirs is read once per process: the session writes it before it
    // starts anything that shows files.
    static const std::unordered_map<std::string, std::string> icons = [] {
        const std::pair<paths::UserDirectory, const char*> folders[] = {
            {paths::UserDirectory::Desktop, "folder-desktop"},
            {paths::UserDirectory::Documents, "folder-documents"},
            {paths::UserDirectory::Downloads, "folder-download"},
            {paths::UserDirectory::Music, "folder-music"},
            {paths::UserDirectory::Pictures, "folder-picture"},
            {paths::UserDirectory::Videos, "folder-video"},
            {paths::UserDirectory::Templates, "folder-templates"},
            {paths::UserDirectory::PublicShare, "folder-publicshare"},
        };

        std::unordered_map<std::string, std::string> result;
        // Home first: disabled XDG directories resolve to the home directory itself.
        result.emplace(internal::cleanPath(paths::home()), "folder-home");
        for (const auto& [location, icon] : folders) {
            const std::string standard = paths::userDirectory(location);
            if (!standard.empty())
                result.emplace(internal::cleanPath(standard), icon);
        }
        return result;
    }();

    const auto found = icons.find(internal::cleanPath(path));
    return found == icons.cend() ? std::string() : found->second;
}

bool isStandardFolder(const Url& url)
{
    return url.isLocalFile() && !standardFolderIconName(url.toLocalFile()).empty();
}

bool canMoveStandardFolderTo(const Url& source, const Url& destination)
{
    if (!isStandardFolder(source))
        return true;

    const std::string home = paths::home();
    if (home.empty() || !source.isLocalFile() || !destination.isLocalFile())
        return false;

    const std::string cleanedHome = internal::cleanPath(home);
    return internal::cleanPath(source.toLocalFile()) != cleanedHome
        && internal::cleanPath(destination.toLocalFile()) == cleanedHome;
}

} // namespace Lingmo::Files
