// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mtpbackend.h"

#include "base/i18n.h"
#include "base/logging.h"
#include "base/paths.h"
#include "base/strings.h"

#include <filesystem>

#include <libmtp.h>

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <linux/fs.h>
#include <unistd.h>

using Lingmo::i18n::tr;

static const Lingmo::base::LogCategory lcMtp("lingmo.mountd.mtp");

static constexpr uint32_t kStorageTop = 0xFFFFFFFF;
// Standard MTP partial reads carry a 32-bit offset; Android's extension a 64-bit one.
static constexpr uint64_t kMaxPartialOffset = 0xFFFFFFFFull;

struct MtpBackend::OpenFile
{
    std::string path;
    Object object;
    bool exists = false;
    bool writable = false;
    bool dirty = false;
    // A local copy: the file being written, or one read whole from a device that cannot
    // read parts. -1 reads straight from the device.
    int fd = -1;
};

static MtpBackend::OpenFile *fileFor(uint64_t handle)
{
    return reinterpret_cast<MtpBackend::OpenFile *>(handle);
}

static std::string parentOf(const std::string &path)
{
    const size_t slash = path.rfind('/');
    return slash == 0 || slash == std::string::npos ? std::string("/") : path.substr(0, slash);
}

static std::string nameOf(const std::string &path)
{
    return path.substr(path.rfind('/') + 1);
}

static std::string childOf(const std::string &folder, const std::string &name)
{
    return folder == "/" ? '/' + name : folder + '/' + name;
}

// Copies go to disk next to the user's cache, not to a memory-backed /tmp: phones hold
// videos many gigabytes large.
static int spoolFile()
{
    const std::string directory = Lingmo::base::paths::cacheHome() + "/lingmo-mountd";
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    return ::open(directory.c_str(), O_TMPFILE | O_RDWR | O_CLOEXEC, 0600);
}

MtpBackend::MtpBackend(uint32_t bus, uint8_t device)
    : m_bus(bus)
    , m_deviceNumber(device)
{
}

MtpBackend::~MtpBackend()
{
    for (const auto &[path, file] : m_writing) {
        if (file->fd >= 0)
            ::close(file->fd);
        delete file;
    }

    if (m_device)
        LIBMTP_Release_Device(m_device);
}

int MtpBackend::lastError(int fallback)
{
    if (m_device) {
        if (LIBMTP_error_t *stack = LIBMTP_Get_Errorstack(m_device)) {
            for (LIBMTP_error_t *error = stack; error; error = error->next)
                LM_WARNING(lcMtp) << "libmtp: " << error->error_text;
        }
        LIBMTP_Clear_Errorstack(m_device);
    }

    return -fallback;
}

ConnectResult MtpBackend::connect(const Credentials &)
{
    ConnectResult result;

    LIBMTP_raw_device_t *devices = nullptr;
    int count = 0;
    LIBMTP_Detect_Raw_Devices(&devices, &count);

    for (int i = 0; i < count && !m_device; ++i) {
        if (devices[i].bus_location == m_bus && devices[i].devnum == m_deviceNumber)
            m_device = LIBMTP_Open_Raw_Device_Uncached(&devices[i]);
    }
    free(devices);

    if (!m_device) {
        result.message = tr("The device could not be opened. Unplug it and connect it again.");
        return result;
    }

    m_partialRead = LIBMTP_Check_Capability(m_device, LIBMTP_DEVICECAP_GetPartialObject);
    m_move = LIBMTP_Check_Capability(m_device, LIBMTP_DEVICECAP_MoveObject);
    // Android offers 64-bit partial reads; some devices have its edit operations without
    // naming the extension.
    for (LIBMTP_device_extension_t *extension = m_device->extensions; extension;
         extension = extension->next) {
        if (extension->name && std::strcmp(extension->name, "android.com") == 0)
            m_android = true;
    }
    m_android = m_android
            || (m_partialRead && LIBMTP_Check_Capability(m_device, LIBMTP_DEVICECAP_SendPartialObject)
                && LIBMTP_Check_Capability(m_device, LIBMTP_DEVICECAP_EditObjects));

    loadStorages();
    if (m_storages.empty()) {
        LIBMTP_Release_Device(m_device);
        m_device = nullptr;
        result.message = tr("The device's files are not available. Unlock it and allow access to its data "
                            "on its screen; if that does not help, choose file transfer in its USB settings.");
        return result;
    }

    m_connected = time(nullptr);
    result.status = ConnectResult::Connected;
    return result;
}

std::string MtpBackend::friendlyName() const
{
    if (!m_device)
        return {};

    char *name = LIBMTP_Get_Friendlyname(m_device);
    if (!name || !*name) {
        free(name);
        name = LIBMTP_Get_Modelname(m_device);
    }

    const std::string result = name ? std::string(Lingmo::base::strings::trimmed(name)) : std::string();
    free(name);
    return result;
}

void MtpBackend::loadStorages()
{
    m_storages.clear();
    if (LIBMTP_Get_Storage(m_device, LIBMTP_STORAGE_SORTBY_NOTSORTED) != 0) {
        lastError();
        return;
    }

    std::map<std::string, int> uses;
    for (LIBMTP_devicestorage_t *storage = m_device->storage; storage; storage = storage->next)
        ++uses[storage->StorageDescription ? storage->StorageDescription : ""];

    // Two storages may share a description, or have none: the id tells them apart.
    for (LIBMTP_devicestorage_t *storage = m_device->storage; storage; storage = storage->next) {
        const std::string description = storage->StorageDescription
                ? storage->StorageDescription
                : "";
        char name[256];
        if (description.empty())
            snprintf(name, sizeof(name), "Storage (%X)", storage->id);
        else if (uses[description] > 1)
            snprintf(name, sizeof(name), "%s (%X)", description.c_str(), storage->id);
        else
            snprintf(name, sizeof(name), "%s", description.c_str());

        std::string folder(name);
        std::replace(folder.begin(), folder.end(), '/', '_');
        m_storages[folder] = storage->id;
    }
}

void MtpBackend::fillStat(const Object &object, struct stat *st) const
{
    std::memset(st, 0, sizeof(*st));
    st->st_mode = object.folder ? (S_IFDIR | 0755) : (S_IFREG | 0644);
    st->st_nlink = object.folder ? 2 : 1;
    st->st_uid = ::getuid();
    st->st_gid = ::getgid();
    st->st_size = object.size;
    st->st_blocks = (object.size + 511) / 512;
    st->st_blksize = 65536;
    const time_t time = object.modified ? object.modified : m_connected;
    st->st_mtim.tv_sec = time;
    st->st_atim.tv_sec = time;
    st->st_ctim.tv_sec = time;
}

void MtpBackend::forget(const std::string &path)
{
    // Everything below goes too; the prefix ends in a slash so /a never takes /ab.
    const std::string prefix = path + '/';
    for (auto it = m_cache.begin(); it != m_cache.end();) {
        if (it->first == path || it->first.compare(0, prefix.size(), prefix) == 0)
            it = m_cache.erase(it);
        else
            ++it;
    }
}

int MtpBackend::listFolder(const std::string &path, const Object &folder,
                           std::vector<DirectoryEntry> *entries)
{
    LIBMTP_file_t *files = LIBMTP_Get_Files_And_Folders(m_device, folder.storage, folder.id);
    if (!files && LIBMTP_Get_Errorstack(m_device))
        return lastError();

    // The children are listed afresh; what was known of them may be stale.
    const std::string prefix = path == "/" ? std::string("/") : path + '/';
    for (auto it = m_cache.lower_bound(prefix); it != m_cache.end()
         && it->first.compare(0, prefix.size(), prefix) == 0;) {
        if (it->first.find('/', prefix.size()) == std::string::npos)
            it = m_cache.erase(it);
        else
            ++it;
    }

    while (files) {
        LIBMTP_file_t *file = files;
        files = files->next;

        if (file->filename && *file->filename && !std::strchr(file->filename, '/')) {
            const std::string name(file->filename);
            const std::string child = childOf(path, name);

            // MTP allows two objects of one name in a folder; the first one is shown.
            if (!m_cache.count(child)) {
                Object object;
                object.storage = file->storage_id;
                object.id = file->item_id;
                object.folder = file->filetype == LIBMTP_FILETYPE_FOLDER;
                object.size = file->filesize;
                object.modified = file->modificationdate;
                m_cache[child] = object;

                if (entries) {
                    DirectoryEntry entry;
                    entry.name = name;
                    fillStat(object, &entry.st);
                    entries->push_back(std::move(entry));
                }
            }
        }

        LIBMTP_destroy_file_t(file);
    }

    return 0;
}

bool MtpBackend::lookup(const std::string &path, Object *object)
{
    if (path == "/") {
        object->folder = true;
        object->id = 0;
        return true;
    }

    if (const auto cached = m_cache.find(path); cached != m_cache.end()) {
        *object = cached->second;
        return true;
    }

    const std::string parent = parentOf(path);
    if (parent == "/") {
        const auto storage = m_storages.find(nameOf(path));
        if (storage == m_storages.end())
            return false;

        object->storage = storage->second;
        object->id = kStorageTop;
        object->folder = true;
        object->size = 0;
        object->modified = 0;
        return true;
    }

    Object folder;
    if (!lookup(parent, &folder) || !folder.folder)
        return false;

    if (listFolder(parent, folder, nullptr) < 0)
        return false;

    const auto found = m_cache.find(path);
    if (found == m_cache.end())
        return false;

    *object = found->second;
    return true;
}

int MtpBackend::stat(const std::string &path, struct stat *st)
{
    if (const auto writing = m_writing.find(path); writing != m_writing.end()) {
        OpenFile *file = writing->second;
        Object object = file->object;
        struct stat local;
        if (::fstat(file->fd, &local) == 0)
            object.size = local.st_size;
        fillStat(object, st);
        return 0;
    }

    Object object;
    if (!lookup(path, &object))
        return -ENOENT;

    fillStat(object, st);
    return 0;
}

int MtpBackend::list(const std::string &path, std::vector<DirectoryEntry> *entries)
{
    if (path == "/") {
        loadStorages();
        for (const auto &[name, id] : m_storages) {
            Object object;
            object.storage = id;
            object.id = kStorageTop;
            object.folder = true;
            DirectoryEntry entry;
            entry.name = name;
            fillStat(object, &entry.st);
            entries->push_back(std::move(entry));
        }
        return 0;
    }

    Object folder;
    if (!lookup(path, &folder))
        return -ENOENT;
    if (!folder.folder)
        return -ENOTDIR;

    if (const int result = listFolder(path, folder, entries); result < 0)
        return result;

    // A new file being written is not on the device yet.
    for (const auto &[writingPath, file] : m_writing) {
        if (file->exists || parentOf(writingPath) != path)
            continue;

        DirectoryEntry entry;
        entry.name = nameOf(writingPath);
        stat(writingPath, &entry.st);
        entries->push_back(std::move(entry));
    }

    return 0;
}

int MtpBackend::download(const Object &object, int fd)
{
    if (LIBMTP_Get_File_To_File_Descriptor(m_device, object.id, fd, nullptr, nullptr) != 0)
        return lastError();

    return ::lseek(fd, 0, SEEK_SET) < 0 ? -errno : 0;
}

int MtpBackend::open(const std::string &path, int flags, mode_t, uint64_t *handle)
{
    const std::string parent = parentOf(path);
    if (path == "/" || parent == "/")
        return (flags & O_CREAT) ? -EPERM : -EISDIR;

    Object parentFolder;
    if (!lookup(parent, &parentFolder))
        return -ENOENT;
    if (!parentFolder.folder)
        return -ENOTDIR;

    Object object;
    const bool exists = lookup(path, &object) || m_writing.count(path);
    if (exists && (flags & O_CREAT) && (flags & O_EXCL))
        return -EEXIST;
    if (!exists && !(flags & O_CREAT))
        return -ENOENT;
    if (exists && object.folder)
        return -EISDIR;

    auto file = std::make_unique<OpenFile>();
    file->path = path;
    file->exists = exists && !m_writing.count(path);
    file->writable = (flags & O_ACCMODE) != O_RDONLY;

    if (file->exists) {
        file->object = object;
    } else {
        file->object.storage = parentFolder.storage;
        file->object.modified = time(nullptr);
    }

    if (file->writable) {
        // A path written twice at once would upload twice; the later writer waits its turn.
        if (m_writing.count(path))
            return -EBUSY;

        file->fd = spoolFile();
        if (file->fd < 0)
            return -errno;

        // A new or emptied file must reach the device even if nothing is written.
        file->dirty = !file->exists || (flags & O_TRUNC);
        if (file->exists && !(flags & O_TRUNC)) {
            if (const int result = download(object, file->fd); result < 0) {
                ::close(file->fd);
                return result;
            }
        }

        m_writing[path] = file.get();
    } else if (!m_partialRead) {
        file->fd = spoolFile();
        if (file->fd < 0)
            return -errno;
        if (const int result = download(object, file->fd); result < 0) {
            ::close(file->fd);
            return result;
        }
    }

    *handle = reinterpret_cast<uint64_t>(file.release());
    return 0;
}

int MtpBackend::read(uint64_t handle, char *buffer, size_t size, off_t offset)
{
    OpenFile *file = fileFor(handle);
    if (file->fd >= 0) {
        const ssize_t count = ::pread(file->fd, buffer, size, offset);
        return count < 0 ? -errno : int(count);
    }

    // Many Android devices hang when asked to read past the end; stop there.
    const uint64_t length = file->object.size;
    if (uint64_t(offset) >= length)
        return 0;
    if (uint64_t(offset) > kMaxPartialOffset && !m_android)
        return -EOVERFLOW;

    const uint32_t wanted = uint32_t(std::min<uint64_t>(size, length - offset));
    unsigned char *data = nullptr;
    unsigned int received = 0;
    if (LIBMTP_GetPartialObject(m_device, file->object.id, offset, wanted, &data, &received) != 0) {
        free(data);
        return lastError();
    }

    if (received > wanted) {
        free(data);
        return -EIO;
    }

    std::memcpy(buffer, data, received);
    free(data);
    return int(received);
}

int MtpBackend::write(uint64_t handle, const char *buffer, size_t size, off_t offset)
{
    OpenFile *file = fileFor(handle);
    if (!file->writable)
        return -EBADF;

    const ssize_t count = ::pwrite(file->fd, buffer, size, offset);
    if (count < 0)
        return -errno;

    file->dirty = true;
    return int(count);
}

// The new object goes up under the same name before the old one is deleted, which MTP
// allows, so a failed upload leaves the old file in place.
int MtpBackend::upload(OpenFile *file)
{
    Object parent;
    if (!lookup(parentOf(file->path), &parent))
        return -ENOENT;

    struct stat local;
    if (::fstat(file->fd, &local) < 0 || ::lseek(file->fd, 0, SEEK_SET) < 0)
        return -errno;

    LIBMTP_file_t *metadata = LIBMTP_new_file_t();
    metadata->filename = strdup(nameOf(file->path).c_str());
    metadata->filesize = local.st_size;
    metadata->filetype = LIBMTP_FILETYPE_UNKNOWN;
    metadata->parent_id = parent.id;
    metadata->storage_id = parent.storage;

    const int sent = LIBMTP_Send_File_From_File_Descriptor(m_device, file->fd, metadata, nullptr,
                                                           nullptr);
    const uint32_t id = metadata->item_id;
    LIBMTP_destroy_file_t(metadata);
    if (sent != 0)
        return lastError();

    if (file->exists && file->object.id != id && LIBMTP_Delete_Object(m_device, file->object.id) != 0)
        lastError();

    file->exists = true;
    file->object.id = id;
    file->object.storage = parent.storage;
    file->object.size = local.st_size;
    file->object.modified = time(nullptr);
    m_cache[file->path] = file->object;
    return 0;
}

int MtpBackend::flush(uint64_t handle)
{
    OpenFile *file = fileFor(handle);
    if (!file->writable || !file->dirty)
        return 0;

    const int result = upload(file);
    if (result == 0)
        file->dirty = false;
    return result;
}

int MtpBackend::release(uint64_t handle)
{
    OpenFile *file = fileFor(handle);
    const int result = flush(handle);

    if (file->writable)
        m_writing.erase(file->path);
    if (file->fd >= 0)
        ::close(file->fd);
    delete file;
    return result;
}

int MtpBackend::truncate(const std::string &path, off_t size, const uint64_t *handle)
{
    if (handle) {
        OpenFile *file = fileFor(*handle);
        if (!file->writable)
            return -EBADF;
        if (::ftruncate(file->fd, size) < 0)
            return -errno;
        file->dirty = true;
        return 0;
    }

    uint64_t opened = 0;
    if (const int result = open(path, size == 0 ? O_WRONLY | O_TRUNC : O_RDWR, 0, &opened);
        result < 0)
        return result;

    OpenFile *file = fileFor(opened);
    if (::ftruncate(file->fd, size) < 0) {
        const int result = -errno;
        file->dirty = false;
        release(opened);
        return result;
    }

    file->dirty = true;
    return release(opened);
}

int MtpBackend::remove(const std::string &path)
{
    Object object;
    if (!lookup(path, &object))
        return -ENOENT;
    if (object.folder)
        return -EISDIR;

    if (LIBMTP_Delete_Object(m_device, object.id) != 0)
        return lastError();

    forget(path);
    return 0;
}

int MtpBackend::makeDirectory(const std::string &path, mode_t)
{
    const std::string parentPath = parentOf(path);
    if (parentPath == "/")
        return -EPERM;

    Object parent;
    if (!lookup(parentPath, &parent))
        return -ENOENT;
    if (!parent.folder)
        return -ENOTDIR;

    Object existing;
    if (lookup(path, &existing))
        return -EEXIST;

    char *name = strdup(nameOf(path).c_str());
    const uint32_t id = LIBMTP_Create_Folder(m_device, name, parent.id, parent.storage);
    free(name);
    // Unlike most libmtp calls, this one fails with 0.
    if (id == 0)
        return lastError();

    Object folder;
    folder.storage = parent.storage;
    folder.id = id;
    folder.folder = true;
    folder.modified = time(nullptr);
    m_cache[path] = folder;
    return 0;
}

int MtpBackend::removeDirectory(const std::string &path)
{
    if (parentOf(path) == "/")
        return -EPERM;

    Object folder;
    if (!lookup(path, &folder))
        return -ENOENT;
    if (!folder.folder)
        return -ENOTDIR;

    // Deleting a folder over MTP takes everything in it.
    LIBMTP_file_t *children = LIBMTP_Get_Files_And_Folders(m_device, folder.storage, folder.id);
    if (children) {
        while (children) {
            LIBMTP_file_t *next = children->next;
            LIBMTP_destroy_file_t(children);
            children = next;
        }
        return -ENOTEMPTY;
    }

    if (LIBMTP_Delete_Object(m_device, folder.id) != 0)
        return lastError();

    forget(path);
    return 0;
}

int MtpBackend::rename(const std::string &from, const std::string &to, unsigned flags)
{
    if (flags & RENAME_EXCHANGE)
        return -EINVAL;
    if (parentOf(from) == "/" || parentOf(to) == "/")
        return -EPERM;

    Object source;
    if (!lookup(from, &source))
        return -ENOENT;

    Object targetFolder;
    if (!lookup(parentOf(to), &targetFolder))
        return -ENOENT;
    if (!targetFolder.folder)
        return -ENOTDIR;

    const bool sameFolder = parentOf(from) == parentOf(to);
    if (!sameFolder && (!m_move || targetFolder.storage != source.storage))
        return -EXDEV;

    Object existing;
    if (lookup(to, &existing) && existing.id != source.id) {
        if (flags & RENAME_NOREPLACE)
            return -EEXIST;
        const int removed = existing.folder ? removeDirectory(to) : remove(to);
        if (removed < 0)
            return removed;
    }

    if (!sameFolder) {
        // Move takes 0, not the top-folder id, for a storage's top.
        const uint32_t parent = targetFolder.id == kStorageTop ? 0 : targetFolder.id;
        if (LIBMTP_Move_Object(m_device, source.id, targetFolder.storage, parent) != 0)
            return lastError();
    }

    const std::string name = nameOf(to);
    if (name != nameOf(from)) {
        int renamed = -1;
        if (source.folder) {
            LIBMTP_folder_t *folder = LIBMTP_new_folder_t();
            folder->folder_id = source.id;
            folder->storage_id = source.storage;
            folder->parent_id = targetFolder.id;
            folder->name = strdup(nameOf(from).c_str());
            renamed = LIBMTP_Set_Folder_Name(m_device, folder, name.c_str());
            LIBMTP_destroy_folder_t(folder);
        } else if (LIBMTP_file_t *file = LIBMTP_Get_Filemetadata(m_device, source.id)) {
            renamed = LIBMTP_Set_File_Name(m_device, file, name.c_str());
            LIBMTP_destroy_file_t(file);
        }

        if (renamed != 0)
            return lastError();
    }

    forget(from);
    forget(to);
    m_cache[to] = source;
    return 0;
}

int MtpBackend::statfs(const std::string &path, struct statvfs *st)
{
    std::memset(st, 0, sizeof(*st));
    st->f_bsize = 4096;
    st->f_frsize = 4096;
    st->f_namemax = 255;

    if (LIBMTP_Get_Storage(m_device, LIBMTP_STORAGE_SORTBY_NOTSORTED) != 0)
        return lastError();

    // A path in a storage counts that storage; the top of the device counts them all.
    uint32_t only = 0;
    if (path != "/") {
        const std::string top = path.substr(1, path.find('/', 1) - 1);
        if (const auto storage = m_storages.find(top); storage != m_storages.end())
            only = storage->second;
    }

    uint64_t capacity = 0;
    uint64_t available = 0;
    for (LIBMTP_devicestorage_t *storage = m_device->storage; storage; storage = storage->next) {
        if (only && storage->id != only)
            continue;
        capacity += storage->MaxCapacity;
        available += storage->FreeSpaceInBytes;
    }

    st->f_blocks = capacity / st->f_frsize;
    st->f_bfree = available / st->f_frsize;
    st->f_bavail = st->f_bfree;
    return 0;
}
