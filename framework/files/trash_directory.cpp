// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trash_directory_p.h"

#include <algorithm>
#include <cerrno>
#include <ctime>
#include <fcntl.h>
#include <mutex>
#include <set>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"
#include "base/url.h"
#include "lingmo/files/error.h"
#include "path_utils_p.h"
#include "posix_p.h"

namespace Lingmo::Files::internal {

using namespace Lingmo::Base;

static std::mutex s_overridesMutex;
static std::string s_mountTable = "/proc/self/mountinfo";
static std::string s_homeTrashRoot;

void setMountTable(const std::string& path)
{
    const std::lock_guard lock(s_overridesMutex);
    s_mountTable = path;
}

static std::string mountTable()
{
    const std::lock_guard lock(s_overridesMutex);
    return s_mountTable;
}

void setHomeTrashRoot(const std::string& path)
{
    const std::lock_guard lock(s_overridesMutex);
    s_homeTrashRoot = path;
}

std::string homeTrashRoot()
{
    {
        const std::lock_guard lock(s_overridesMutex);
        if (!s_homeTrashRoot.empty())
            return s_homeTrashRoot;
    }

    return paths::dataHome() + "/Trash";
}

static std::string userId()
{
    return strings::number(::getuid());
}

TrashDirectory homeTrash()
{
    return {"home", homeTrashRoot(), {}};
}

TrashDirectory volumeTrash(const std::string& topdir, bool shared)
{
    const std::string id = (shared ? "s" : "u") + strings::toHex(topdir);
    const std::string path = shared ? joined(topdir, ".Trash/" + userId()) : joined(topdir, ".Trash-" + userId());
    return {id, path, topdir};
}

static TrashDirectory trashForId(const std::string& id)
{
    if (id == "home")
        return homeTrash();

    const std::string hex = id.substr(std::min<size_t>(1, id.size()));
    const bool valid = (id.starts_with('s') || id.starts_with('u')) && !hex.empty() && hex.size() % 2 == 0
        && std::all_of(hex.cbegin(), hex.cend(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
    if (!valid)
        throw Failure(EINVAL, id);

    const std::string topdir = strings::fromHex(hex).value_or(std::string());
    const std::vector<std::string> parts = strings::split(topdir, '/');
    if (topdir.find('\0') != std::string::npos || !topdir.starts_with('/') || cleanPath(topdir) != topdir
        || std::find(parts.cbegin(), parts.cend(), "..") != parts.cend())
        throw Failure(EINVAL, id);

    return volumeTrash(topdir, id.starts_with('s'));
}

TrashDirectory trashDirectory(const Url& url, std::string* inside)
{
    const std::string path = url.path().substr(std::min<size_t>(1, url.path().size()));
    const size_t slash = path.find('/');
    const std::string id = path.substr(0, slash);

    if (url.scheme() != "trash" || id.empty())
        throw Failure(EINVAL, url.toString());

    if (inside)
        *inside = slash == std::string::npos ? std::string() : path.substr(slash + 1);

    return trashForId(id);
}

// Kernel interfaces, memory-backed folders, automounts and remote filesystems: a
// memory-backed trash would be lost at shutdown, so its files go home instead;
// looking into an automount mounts it, and a hung share would block the listing and
// its watch.
static bool holdsNoTrash(const std::string& mountPoint, const std::string& type)
{
    // The system's own places; removable volumes may be mounted under /run/media.
    for (const std::string_view system : {"/proc", "/sys", "/dev", "/run"}) {
        if (mountPoint.starts_with(system) && (mountPoint.size() == system.size() || mountPoint[system.size()] == '/'))
            return !mountPoint.starts_with("/run/media/");
    }

    static const std::set<std::string, std::less<>> types = {
        "proc",     "sysfs",     "devtmpfs",   "devpts",      "cgroup",
        "cgroup2",  "debugfs",   "tracefs",    "securityfs",  "bpf",
        "pstore",   "autofs",    "mqueue",     "hugetlbfs",   "configfs",
        "fusectl",  "efivarfs",  "nsfs",       "binfmt_misc", "rpc_pipefs",
        "tmpfs",    "ramfs",
        "fuse.portal", "fuse.gvfsd-fuse", "fuse.lingmo-mountd",
        "nfs",      "nfs4",      "cifs",       "smb3",        "smbfs",
        "ncpfs",    "afs",       "ceph",       "9p",          "glusterfs",
        "coda",     "davfs",     "fuse.sshfs", "fuse.rclone", "fuse.davfs2",
    };

    return types.contains(type);
}

// Mount points in the table escape space, tab, newline and backslash as octal.
static std::string unescapeMountPoint(std::string_view field)
{
    std::string result;
    result.reserve(field.size());

    const auto octal = [&](size_t i) {
        return i < field.size() && field[i] >= '0' && field[i] <= '7';
    };

    for (size_t i = 0; i < field.size(); ++i) {
        if (field[i] == '\\' && octal(i + 1) && octal(i + 2) && octal(i + 3)) {
            result += char((field[i + 1] - '0') * 64 + (field[i + 2] - '0') * 8 + (field[i + 3] - '0'));
            i += 3;
        } else {
            result += field[i];
        }
    }

    return result;
}

// Each mount point with the type mounted there last, which hides any below it.
static std::vector<std::pair<std::string, std::string>> mounts()
{
    const std::optional<std::string> table = file::read(mountTable());
    if (!table)
        return {};

    std::vector<std::pair<std::string, std::string>> result;

    for (const std::string_view line : strings::splitViews(*table, '\n')) {
        // ID, parent, device, root, mount point, options, optional fields, "-",
        // type, source, super options.
        const std::vector<std::string_view> fields = strings::splitViews(line, ' ');
        size_t separator = std::string_view::npos;
        for (size_t i = 6; i < fields.size(); ++i) {
            if (fields[i] == "-") {
                separator = i;
                break;
            }
        }
        if (fields.size() < 5 || separator == std::string_view::npos || separator + 1 >= fields.size())
            continue;

        const std::string mountPoint = unescapeMountPoint(fields[4]);
        if (!mountPoint.starts_with('/'))
            continue;

        const auto listed = std::find_if(result.begin(), result.end(), [&](const auto& mount) {
            return mount.first == mountPoint;
        });
        if (listed != result.end())
            listed->second = std::string(fields[separator + 1]);
        else
            result.emplace_back(mountPoint, std::string(fields[separator + 1]));
    }

    return result;
}

static std::vector<std::string> mountPoints()
{
    std::vector<std::string> result;

    for (const auto& [mountPoint, type] : mounts()) {
        // A volume the user cannot enter, such as a service's credentials, has no
        // trash of theirs.
        if (!holdsNoTrash(mountPoint, type) && ::access(mountPoint.c_str(), X_OK) == 0)
            result.push_back(mountPoint);
    }

    return result;
}

bool volumeTakesTrash(const std::string& topdir)
{
    for (const auto& [mountPoint, type] : mounts()) {
        if (mountPoint == topdir)
            return !holdsNoTrash(mountPoint, type);
    }

    return true;
}

static bool isOwnDirectory(const std::string& path, struct stat* st)
{
    return ::lstat(path.c_str(), st) == 0 && S_ISDIR(st->st_mode) && st->st_uid == ::getuid();
}

// Per the trash specification, $topdir/.Trash is used only when it is a sticky
// directory and not a link, since every user creates their own folder in it.
static bool isSharedTrashUsable(const std::string& topdir)
{
    struct stat st;
    return ::lstat(joined(topdir, ".Trash").c_str(), &st) == 0 && S_ISDIR(st.st_mode) && (st.st_mode & S_ISVTX);
}

// Creates path as a folder of the user's own, or checks the one there.
static bool ensureOwnDirectory(const std::string& path)
{
    struct stat st;
    if (::mkdir(path.c_str(), 0700) < 0 && errno != EEXIST)
        return false;

    return isOwnDirectory(path, &st);
}

bool prepareVolumeTrash(const TrashDirectory& trash)
{
    if (trash.id.starts_with('s') && !isSharedTrashUsable(trash.topdir))
        return false;

    // A read-only medium keeps the folders it has but takes nothing.
    return ensureOwnDirectory(trash.path) && ensureOwnDirectory(trash.path + "/files")
        && ensureOwnDirectory(trash.path + "/info") && ::access((trash.path + "/files").c_str(), W_OK) == 0
        && ::access((trash.path + "/info").c_str(), W_OK) == 0;
}

std::vector<TrashDirectory> trashDirectories(std::vector<std::string>* mountPointsLookedIn)
{
    std::vector<TrashDirectory> result{homeTrash()};
    std::set<std::pair<dev_t, ino_t>> seen;

    struct stat st;
    if (::stat(result.front().path.c_str(), &st) == 0)
        seen.insert({st.st_dev, st.st_ino});

    const std::vector<std::string> points = mountPoints();
    if (mountPointsLookedIn)
        *mountPointsLookedIn = points;

    for (const std::string& topdir : points) {
        for (const bool shared : {true, false}) {
            if (shared && !isSharedTrashUsable(topdir))
                continue;

            const TrashDirectory trash = volumeTrash(topdir, shared);
            // A volume mounted twice, or bound into another place, has one trash.
            if (isOwnDirectory(trash.path, &st) && !seen.contains({st.st_dev, st.st_ino})) {
                seen.insert({st.st_dev, st.st_ino});
                result.push_back(trash);
            }
        }
    }

    return result;
}

static bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

void trashWatches(std::vector<std::string>* paths, WatchEntries* entries)
{
    std::vector<std::string> points;

    for (const TrashDirectory& trash : trashDirectories(&points)) {
        const std::string files = trash.path + "/files";
        if (isDirectory(files))
            paths->push_back(files);
        else
            (*entries)[trash.path].push_back("files");

        (*entries)[absolutePathOf(trash.path)].push_back(fileNameOf(trash.path));
    }

    const std::string user = ".Trash-" + userId();
    for (const std::string& topdir : points) {
        // Where the user cannot make a trash, such as /, one an administrator makes
        // later is found by the next listing.
        if (::access(topdir.c_str(), W_OK) == 0) {
            auto& names = (*entries)[topdir];
            names.push_back(".Trash");
            names.push_back(user);
        }

        if (isDirectory(joined(topdir, ".Trash")))
            (*entries)[joined(topdir, ".Trash")].push_back(userId());
    }
}

// Paths are absolute, or in a volume's trash relative to its mount point. A volume's
// trash is the medium's to fill, so its paths name no place outside the volume.
static std::string resolvedPath(const TrashDirectory& trash, const std::string& path)
{
    if (path.empty() || path.find('\0') != std::string::npos)
        return {};

    if (!path.starts_with('/')) {
        const std::vector<std::string> parts = strings::split(path, '/');
        if (trash.topdir.empty() || std::find(parts.cbegin(), parts.cend(), "..") != parts.cend())
            return {};

        return resolvedPath(trash, cleanPath(joined(trash.topdir, path)));
    }

    const std::string clean = cleanPath(path);
    if (clean == "/")
        return {};

    const std::string top = cleanPath(trash.topdir);
    if (!top.empty() && top != "/" && !clean.starts_with(top + '/'))
        return {};

    return clean;
}

static std::optional<int> digits(std::string_view text, size_t& at, size_t count)
{
    if (at + count > text.size())
        return std::nullopt;

    int value = 0;
    for (size_t i = 0; i < count; ++i) {
        const char c = text[at + i];
        if (c < '0' || c > '9')
            return std::nullopt;
        value = value * 10 + (c - '0');
    }

    at += count;
    return value;
}

// An ISO 8601 time, "yyyy-MM-ddTHH:mm[:ss[.zzz]]", in
// local time, or with "Z" or an offset after it.
static std::optional<int64_t> parseIsoDate(std::string_view text)
{
    size_t at = 0;
    const auto year = digits(text, at, 4);
    if (!year || at >= text.size() || text[at++] != '-')
        return std::nullopt;
    const auto month = digits(text, at, 2);
    if (!month || at >= text.size() || text[at++] != '-')
        return std::nullopt;
    const auto day = digits(text, at, 2);
    if (!day || at >= text.size() || (text[at] != 'T' && text[at] != ' '))
        return std::nullopt;
    ++at;
    const auto hour = digits(text, at, 2);
    if (!hour || at >= text.size() || text[at++] != ':')
        return std::nullopt;
    const auto minute = digits(text, at, 2);
    if (!minute)
        return std::nullopt;

    int second = 0;
    int millisecond = 0;
    if (at < text.size() && text[at] == ':') {
        ++at;
        const auto value = digits(text, at, 2);
        if (!value)
            return std::nullopt;
        second = *value;

        if (at < text.size() && (text[at] == '.' || text[at] == ',')) {
            ++at;
            int scale = 100;
            if (at >= text.size() || text[at] < '0' || text[at] > '9')
                return std::nullopt;
            while (at < text.size() && text[at] >= '0' && text[at] <= '9') {
                millisecond += (text[at] - '0') * scale;
                scale /= 10;
                ++at;
            }
        }
    }

    if (*month < 1 || *month > 12 || *day < 1 || *day > 31 || *hour > 23 || *minute > 59 || second > 59)
        return std::nullopt;

    struct tm fields = {};
    fields.tm_year = *year - 1900;
    fields.tm_mon = *month - 1;
    fields.tm_mday = *day;
    fields.tm_hour = *hour;
    fields.tm_min = *minute;
    fields.tm_sec = second;

    time_t seconds;
    if (at == text.size()) {
        fields.tm_isdst = -1;
        seconds = ::mktime(&fields);
    } else if (text[at] == 'Z' && at + 1 == text.size()) {
        seconds = ::timegm(&fields);
    } else if (text[at] == '+' || text[at] == '-') {
        const int sign = text[at++] == '-' ? -1 : 1;
        const auto offsetHours = digits(text, at, 2);
        if (!offsetHours)
            return std::nullopt;
        if (at < text.size() && text[at] == ':')
            ++at;
        const auto offsetMinutes = at < text.size() ? digits(text, at, 2) : std::optional<int>(0);
        if (!offsetMinutes || at != text.size())
            return std::nullopt;
        seconds = ::timegm(&fields) - sign * (*offsetHours * 3600 + *offsetMinutes * 60);
    } else {
        return std::nullopt;
    }

    return int64_t(seconds) * 1000 + millisecond;
}

std::optional<std::string> readTrashFile(const std::string& path)
{
    constexpr size_t Limit = 1024 * 1024;

    Fd fd(::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC));
    struct stat st;
    if (fd.value < 0 || ::fstat(fd.value, &st) < 0 || !S_ISREG(st.st_mode))
        return std::nullopt;

    std::string contents;
    char buffer[16384];
    while (contents.size() < Limit) {
        const ssize_t size = ::read(fd.value, buffer, std::min(sizeof(buffer), Limit - contents.size()));
        if (size < 0 && errno == EINTR)
            continue;
        if (size < 0)
            return std::nullopt;
        if (size == 0)
            break;
        contents.append(buffer, size_t(size));
    }

    return contents;
}

bool readTrashInfo(const TrashDirectory& trash, const std::string& name, TrashInfo* info)
{
    const std::optional<std::string> contents = readTrashFile(trash.path + "/info/" + name + ".trashinfo");
    if (!contents)
        return false;

    bool group = false;

    for (const std::string_view rawLine : strings::splitViews(*contents, '\n')) {
        const std::string_view line = strings::trimmed(rawLine);
        if (line.starts_with('['))
            group = line == "[Trash Info]";
        else if (group && line.starts_with("Path="))
            info->path = resolvedPath(trash, url::percentDecoded(line.substr(5)));
        else if (group && line.starts_with("DeletionDate="))
            info->deletionTime = parseIsoDate(line.substr(13));
    }

    return true;
}

} // namespace Lingmo::Files::internal
