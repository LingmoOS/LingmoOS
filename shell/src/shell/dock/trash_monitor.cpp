// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/trash_monitor.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <set>
#include <string_view>

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"

namespace Lingmo::Shell {

namespace fs = std::filesystem;

static constexpr std::chrono::milliseconds kDebounce(40);
static constexpr std::chrono::milliseconds kRetry(2000);
static constexpr std::chrono::milliseconds kRetryLimit(60000);

// Kernel interfaces, memory-backed folders, automounts and remote
// filesystems: a memory-backed trash would be lost at shutdown, so the file
// manager sends those files home; looking into an automount mounts it, and a
// hung share would block the look and its watch.
static bool holdsNoTrash(const std::string &mountPoint, const std::string &type)
{
    // The system's own places; removable volumes may be mounted under /run/media.
    for (const std::string_view system : {"/proc", "/sys", "/dev", "/run"}) {
        if (mountPoint.starts_with(system) && (mountPoint.size() == system.size() || mountPoint[system.size()] == '/'))
            return !mountPoint.starts_with("/run/media/");
    }

    static const std::set<std::string> types = {
            "proc",        "sysfs",           "devtmpfs",           "devpts",   "cgroup",      "cgroup2",
            "debugfs",     "tracefs",         "securityfs",         "bpf",      "pstore",      "autofs",
            "mqueue",      "hugetlbfs",       "configfs",           "fusectl",  "efivarfs",    "nsfs",
            "binfmt_misc", "rpc_pipefs",      "fuse.portal",        "fuse.gvfsd-fuse",         "fuse.lingmo-mountd",
            "tmpfs",       "ramfs",
            "nfs",         "nfs4",            "cifs",               "smb3",     "smbfs",       "ncpfs",
            "afs",         "ceph",            "9p",                 "glusterfs", "coda",       "davfs",
            "fuse.sshfs",  "fuse.rclone",     "fuse.davfs2",
    };
    return types.count(type) > 0;
}

// Mount points in the table escape space, tab, newline and backslash as octal.
static std::string unescapeMountPoint(const std::string &field)
{
    std::string result;
    const auto octal = [&field](size_t i) { return i < field.size() && field[i] >= '0' && field[i] <= '7'; };
    for (size_t i = 0; i < field.size(); ++i) {
        if (field[i] == '\\' && octal(i + 1) && octal(i + 2) && octal(i + 3)) {
            result.push_back(char((field[i + 1] - '0') * 64 + (field[i + 2] - '0') * 8 + (field[i + 3] - '0')));
            i += 3;
        } else {
            result.push_back(field[i]);
        }
    }
    return result;
}

// Each mount point that can hold a trash, with the type mounted there last,
// which hides any below it.
static std::vector<std::string> mountPoints(const std::string &mountTable)
{
    const std::optional<std::string> table = base::file::read(mountTable);
    if (!table)
        return {};

    std::vector<std::pair<std::string, std::string>> mounts;
    for (const std::string &line : base::strings::split(*table, '\n')) {
        // ID, parent, device, root, mount point, options, optional fields,
        // "-", type, source, super options.
        const std::vector<std::string> fields = base::strings::split(line, ' ');
        const auto separator = std::find(fields.begin() + std::min<ptrdiff_t>(6, ptrdiff_t(fields.size())),
                                         fields.end(), "-");
        if (fields.size() < 5 || separator == fields.end() || separator + 1 == fields.end())
            continue;
        const std::string mountPoint = unescapeMountPoint(fields[4]);
        if (!mountPoint.starts_with('/'))
            continue;
        const auto listed = std::find_if(mounts.begin(), mounts.end(),
                                         [&mountPoint](const auto &mount) { return mount.first == mountPoint; });
        if (listed != mounts.end())
            listed->second = *(separator + 1);
        else
            mounts.emplace_back(mountPoint, *(separator + 1));
    }

    std::vector<std::string> points;
    for (const auto &[mountPoint, type] : mounts) {
        // A volume the user cannot enter, such as a service's credentials,
        // has no trash of theirs.
        if (!holdsNoTrash(mountPoint, type) && access(mountPoint.c_str(), X_OK) == 0)
            points.push_back(mountPoint);
    }
    return points;
}

static std::string joined(const std::string &directory, const std::string &name)
{
    return directory.ends_with('/') ? directory + name : directory + '/' + name;
}

static bool isOwnDirectory(const std::string &path, struct stat *st)
{
    return lstat(path.c_str(), st) == 0 && S_ISDIR(st->st_mode) && st->st_uid == getuid();
}

// Per the trash specification, $topdir/.Trash is used only when it is a
// sticky directory and not a link, since every user makes a folder in it.
static bool isSharedTrashUsable(const std::string &topdir)
{
    struct stat st;
    return lstat(joined(topdir, ".Trash").c_str(), &st) == 0 && S_ISDIR(st.st_mode) && (st.st_mode & S_ISVTX);
}

TrashMonitor::State TrashMonitor::look(const std::string &mountTable, const std::string &homeTrash)
{
    const std::string uid = std::to_string(getuid());
    std::vector<std::string> trashes{homeTrash};
    std::set<std::pair<dev_t, ino_t>> seen;
    struct stat st;
    if (stat(homeTrash.c_str(), &st) == 0)
        seen.insert({st.st_dev, st.st_ino});

    const std::vector<std::string> points = mountPoints(mountTable);
    for (const std::string &topdir : points) {
        for (const bool shared : {true, false}) {
            if (shared && !isSharedTrashUsable(topdir))
                continue;
            const std::string trash = shared ? joined(topdir, ".Trash/" + uid) : joined(topdir, ".Trash-" + uid);
            // A volume mounted twice, or bound into another place, has one trash.
            if (isOwnDirectory(trash, &st) && !seen.count({st.st_dev, st.st_ino})) {
                seen.insert({st.st_dev, st.st_ino});
                trashes.push_back(trash);
            }
        }
    }

    State state;
    std::error_code error;
    for (const std::string &trash : trashes) {
        const std::string files = trash + "/files";
        if (fs::is_directory(files, error))
            state.paths.push_back(files);
        else
            state.entries[trash].push_back("files");
        const fs::path directory(trash);
        state.entries[directory.parent_path().string()].push_back(directory.filename().string());

        if (state.empty && fs::is_directory(files, error)) {
            fs::directory_iterator it(files, error);
            if (!error && it != fs::directory_iterator())
                state.empty = false;
        }
    }
    for (const std::string &topdir : points) {
        // Where the user cannot make a trash, such as /, one an administrator
        // makes later is found by the next look.
        if (access(topdir.c_str(), W_OK) == 0) {
            state.entries[topdir].push_back(".Trash");
            state.entries[topdir].push_back(".Trash-" + uid);
        }
        if (fs::is_directory(joined(topdir, ".Trash"), error))
            state.entries[joined(topdir, ".Trash")].push_back(uid);
    }
    return state;
}

TrashMonitor::TrashMonitor(std::string mountTable, std::string homeTrash)
    : m_mountTable(std::move(mountTable))
    , m_homeTrash(homeTrash.empty() ? base::paths::dataHome() + "/Trash" : std::move(homeTrash))
    , m_retryDelay(kRetry)
{
    m_debounce.setSingleShot(true);
    m_debounce.onTimeout = [this] { refresh(); };
    m_retry.setSingleShot(true);
    m_retry.onTimeout = [this] { retryWatches(); };

    // Mounts and unmounts change which volumes have a trash; procfs tells
    // through POLLPRI, a test's own table cannot.
    if (m_mountTable.starts_with("/proc/"))
        m_mountFd = open(m_mountTable.c_str(), O_RDONLY | O_CLOEXEC);
    if (m_mountFd >= 0) {
        m_mountWatch = base::EventLoop::current()->watch(m_mountFd, EPOLLPRI, [this](uint32_t) {
            if (!m_debounce.isActive())
                m_debounce.start(kDebounce);
        });
    }

    refresh();
}

TrashMonitor::~TrashMonitor()
{
    if (m_mountWatch)
        base::EventLoop::current()->unwatch(m_mountWatch);
    if (m_mountFd >= 0)
        close(m_mountFd);
}

void TrashMonitor::refresh()
{
    if (m_look.isPending()) {
        m_pending = true;
        return;
    }
    m_look = base::runInThread([mountTable = m_mountTable, homeTrash = m_homeTrash] { return look(mountTable, homeTrash); },
                               [this](State state) {
                                   watch(state);
                                   if (state.empty != m_empty) {
                                       m_empty = state.empty;
                                       emptyChanged();
                                   }
                                   if (std::exchange(m_pending, false))
                                       refresh();
                               });
}

void TrashMonitor::watch(const State &state)
{
    m_watches.clear();
    m_unwatched.clear();
    for (const std::string &path : state.paths) {
        if (!addWatch(path, {}))
            m_unwatched.push_back({path, {}});
    }
    for (const auto &[directory, names] : state.entries) {
        std::error_code error;
        // A trash's own folder may not be there yet; its parent tells.
        if (!fs::is_directory(directory, error))
            continue;
        if (!addWatch(directory, names))
            m_unwatched.push_back({directory, names});
    }

    if (m_unwatched.empty()) {
        m_retry.stop();
        m_retryDelay = kRetry;
    } else if (!m_retry.isActive()) {
        m_retry.start(m_retryDelay);
    }
}

// Watching takes read access, which the user may lack and retrying cannot
// bring; such a directory counts as watched.
bool TrashMonitor::addWatch(const std::string &directory, const std::vector<std::string> &names)
{
    if (access(directory.c_str(), R_OK) != 0)
        return errno == EACCES;

    base::FileWatch watch = base::FileWatch::directory(directory, [this, names](const std::string &name) {
        if (names.empty() || name.empty() || std::find(names.begin(), names.end(), name) != names.end()) {
            if (!m_debounce.isActive())
                m_debounce.start(kDebounce);
        }
    });
    if (!watch.isActive())
        return false;
    m_watches.push_back(std::move(watch));
    return true;
}

void TrashMonitor::retryWatches()
{
    bool changed = false;
    std::vector<Unwatched> unwatched;
    for (Unwatched &entry : m_unwatched) {
        std::error_code error;
        if (!fs::is_directory(entry.directory, error) || addWatch(entry.directory, entry.names))
            changed = true;
        else
            unwatched.push_back(std::move(entry));
    }
    m_unwatched = std::move(unwatched);

    // What changed while it was not watched, or a folder gone since the
    // look, is seen by looking again.
    if (changed && !m_debounce.isActive())
        m_debounce.start(kDebounce);

    if (m_unwatched.empty()) {
        m_retryDelay = kRetry;
    } else {
        m_retryDelay = std::min(m_retryDelay * 2, kRetryLimit);
        m_retry.start(m_retryDelay);
    }
}

static void callFileManager(base::dbus::Connection *bus, const std::string &method,
                            const std::vector<std::string> *paths)
{
    if (!bus)
        return;
    base::dbus::Message message =
            bus->newMethodCall("org.lingmo.FileManager", "/FileManager", "org.lingmo.FileManager", method);
    if (paths)
        message.append(*paths);
    bus->send(message);
}

void TrashMonitor::moveToTrash(const std::vector<std::string> &paths, base::dbus::Connection *bus)
{
    if (!paths.empty())
        callFileManager(bus, "moveToTrash", &paths);
}

void TrashMonitor::emptyTrash(base::dbus::Connection *bus)
{
    callFileManager(bus, "emptyTrash", nullptr);
}

void TrashMonitor::openTrash(base::dbus::Connection *bus)
{
    const std::vector<std::string> trash{"trash:///"};
    callFileManager(bus, "openFiles", &trash);
}

} // namespace Lingmo::Shell
