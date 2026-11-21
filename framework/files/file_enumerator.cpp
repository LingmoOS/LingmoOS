// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/file_enumerator.h"

#include <algorithm>
#include <deque>
#include <linux/magic.h>
#include <optional>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>

#include "async_p.h"
#include "base/strings.h"
#include "lingmo/files/file_system_url.h"
#include "posix_p.h"
#include "trash_directory_p.h"

namespace Lingmo::Files {

using namespace internal;

static uint64_t sizeAt(int parent, const std::string& name, const std::string& path, OperationContext& ctx)
{
    ctx.checkpoint(path);

    struct stat st;
    check(::fstatat(parent, name.c_str(), &st, AT_SYMLINK_NOFOLLOW), path);

    uint64_t size = 0;

    if (S_ISDIR(st.st_mode)) {
        Fd fd(::openat(parent, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        check(fd.value, path);

        children(fd.value, path, [&](const std::string& child, const std::string& childPath) {
            size += sizeAt(fd.value, child, childPath, ctx);
        });
    } else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
        size = uint64_t(st.st_size);
        if (S_ISREG(st.st_mode))
            ctx.allocated += std::min<uint64_t>(uint64_t(st.st_size), uint64_t(st.st_blocks) * 512);
        ctx.advance(size);
    }

    ++ctx.files;
    return size;
}

// Kernel interfaces, whose trees are huge or never end and hold no user files.
static bool isKernelFileSystem(int fd)
{
    struct statfs fs;
    if (::fstatfs(fd, &fs) < 0)
        return false;

    switch (fs.f_type) {
    case PROC_SUPER_MAGIC:
    case SYSFS_MAGIC:
    case DEVPTS_SUPER_MAGIC:
    case CGROUP_SUPER_MAGIC:
    case CGROUP2_SUPER_MAGIC:
    case DEBUGFS_MAGIC:
    case TRACEFS_MAGIC:
    case SECURITYFS_MAGIC:
    case BPF_FS_MAGIC:
    case PSTOREFS_MAGIC:
        return true;
    default:
        return false;
    }
}

FileItemList FileEnumerator::list(const Url& url, bool hidden, OperationContext* context)
{
    if (context)
        context->checkpoint();

    // The folders listed together: trash:/ is the files/ of every trash.
    struct Folder
    {
        std::string path;
        Url url;
        // Set for the files/ of a trash, whose items have info files.
        std::optional<TrashDirectory> trash;
        std::vector<std::string> names;
        std::string urlBase;
        std::string pathBase;
    };

    std::vector<Folder> folders;
    const bool trashRoot = url.scheme() == "trash" && url.path() == "/";

    if (trashRoot) {
        FileSystemUrl validated(url);

        for (const auto& trash : trashDirectories()) {
            Url folderUrl = url;
            folderUrl.setPath('/' + trash.id);
            folders.push_back({trash.path + "/files", folderUrl, trash, {}, {}, {}});
        }
    } else {
        folders.push_back({FileSystemUrl(url).localPath(), url, std::nullopt, {}, {}, {}});

        if (url.scheme() == "trash") {
            std::string inside;
            const auto trash = trashDirectory(url, &inside);
            if (inside.empty())
                folders.front().trash = trash;
        }
    }

    for (auto& folder : folders) {
        const auto& path = folder.path;
        // Nothing trashed yet is no error, and a volume's trash that cannot be read
        // leaves the rest of the trash.
        const bool skippable = trashRoot && !folder.trash->topdir.empty();

        Fd fd(::open(native(path).c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
        if (fd.value < 0 && trashRoot && (errno == ENOENT || skippable))
            continue;
        check(fd.value, path);

        try {
            children(fd.value, path, [&](const std::string& name) {
                if (context)
                    context->checkpoint(path);

                if (!hidden && name.starts_with('.'))
                    return;

                folder.names.push_back(name);
            });
        } catch (const Failure& failure) {
            if (!skippable || failure.error.code == ECANCELED)
                throw;

            folder.names.clear();
        }
    }

    // Where each folder's items start in the listing.
    std::vector<ptrdiff_t> starts;
    ptrdiff_t count = 0;
    for (auto& folder : folders) {
        starts.push_back(count);
        count += ptrdiff_t(folder.names.size());
        folder.urlBase = folder.url.path() + (folder.url.path().ends_with('/') ? "" : "/");
        folder.pathBase = folder.path + (folder.path.ends_with('/') ? "" : "/");
    }

    // Building the items costs more than reading the names: a large folder spreads it.
    constexpr ptrdiff_t BlockSize = 256;
    const ptrdiff_t blocks = (count + BlockSize - 1) / BlockSize;
    std::atomic<ptrdiff_t> nextBlock = 0;

    FileItemList result(static_cast<size_t>(count));
    FileItem* const items = result.data();

    runOnFreeThreads(count >= LargeFolder ? blockHelpers(blocks) : 0, [&](const std::atomic_bool& stop) {
        for (ptrdiff_t block = nextBlock++; block < blocks && !stop; block = nextBlock++) {
            if (context)
                context->checkpoint();

            const ptrdiff_t end = std::min(count, (block + 1) * BlockSize);
            for (ptrdiff_t index = block * BlockSize; index < end; ++index) {
                const auto at = std::upper_bound(starts.cbegin(), starts.cend(), index) - 1;
                const Folder& folder = folders[size_t(at - starts.cbegin())];

                const std::string& name = folder.names[size_t(index - *at)];
                Url child = folder.url;
                child.setPath(folder.urlBase + name);

                FileItem& item = items[index];
                item = FileItem(child, folder.pathBase + name, name);

                // A small file each; the trash is listed seldom.
                TrashInfo info;
                if (folder.trash && readTrashInfo(*folder.trash, name, &info)) {
                    item.d->originalPath = info.path;
                    item.d->deletionTime = info.deletionTime;
                }
            }
        }
    });

    return result;
}

// Unicode default case folding, for case-insensitive comparison.
static std::u16string folded(std::string_view text)
{
    const std::u16string source = base::strings::utf8ToUtf16(text);
    std::u16string result(source.size() + 8, u'\0');
    UErrorCode error = U_ZERO_ERROR;
    int32_t length = u_strFoldCase(reinterpret_cast<UChar*>(result.data()), int32_t(result.size()),
                                   reinterpret_cast<const UChar*>(source.data()), int32_t(source.size()),
                                   U_FOLD_CASE_DEFAULT, &error);
    if (error == U_BUFFER_OVERFLOW_ERROR) {
        result.assign(size_t(length), u'\0');
        error = U_ZERO_ERROR;
        length = u_strFoldCase(reinterpret_cast<UChar*>(result.data()), int32_t(result.size()),
                               reinterpret_cast<const UChar*>(source.data()), int32_t(source.size()),
                               U_FOLD_CASE_DEFAULT, &error);
    }
    if (U_FAILURE(error))
        return source;

    result.resize(size_t(length));
    return result;
}

FileItemList FileEnumerator::search(const Url& url, const std::vector<std::string>& words, bool hidden,
                                    OperationContext* context, const std::function<void(FileItem&)>& found)
{
    struct Folder
    {
        Url url;
        std::string path;
    };

    std::vector<std::u16string> foldedWords;
    for (const std::string& word : words)
        foldedWords.push_back(folded(word));

    FileItemList result;
    FileItem::TypeDetails details;
    std::deque<Folder> folders;
    folders.push_back({url, FileSystemUrl(url).localPath()});
    bool first = true;

    while (!folders.empty()) {
        const Folder folder = std::move(folders.front());
        folders.pop_front();

        if (context)
            context->checkpoint();

        const int fd = ::open(native(folder.path).c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (fd < 0 && first)
            check(fd, folder.path);
        first = false;
        if (fd < 0)
            continue;

        if (isKernelFileSystem(fd)) {
            ::close(fd);
            continue;
        }

        DIR* raw = ::fdopendir(fd);
        if (!raw) {
            ::close(fd);
            continue;
        }

        auto closeDir = [](DIR* value) {
            ::closedir(value);
        };
        std::unique_ptr<DIR, decltype(closeDir)> dir(raw, closeDir);

        const std::string urlBase = folder.url.path() + (folder.url.path().ends_with('/') ? "" : "/");
        const std::string pathBase = folder.path + (folder.path.ends_with('/') ? "" : "/");

        // A folder that fails halfway keeps what it listed.
        while (auto* entry = ::readdir(dir.get())) {
            if (context)
                context->checkpoint();

            const std::string name(entry->d_name);
            if (name == "." || name == ".." || (!hidden && name.starts_with('.')))
                continue;

            bool isDir = entry->d_type == DT_DIR;
            if (entry->d_type == DT_UNKNOWN) {
                struct stat st;
                isDir = ::fstatat(::dirfd(dir.get()), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) == 0
                    && S_ISDIR(st.st_mode);
            }

            Url child = folder.url;
            child.setPath(urlBase + name);

            if (isDir)
                folders.push_back({child, pathBase + name});

            const std::u16string foldedName = folded(name);
            const bool matches = std::all_of(foldedWords.cbegin(), foldedWords.cend(), [&](const std::u16string& word) {
                return foldedName.find(word) != std::u16string::npos;
            });
            if (!matches)
                continue;

            FileItem item(child, pathBase + name, name);
            item.prepareMetadata(FileItem(), &details);
            if (found)
                found(item);
            result.push_back(std::move(item));
        }
    }

    return result;
}

uint64_t FileEnumerator::size(const std::string& path, OperationContext& ctx)
{
    if (path == "/")
        return sizeAt(AT_FDCWD, "/", path, ctx);

    Parent parent(path);
    return sizeAt(parent.fd.value, parent.name, path, ctx);
}

} // namespace Lingmo::Files
