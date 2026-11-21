// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/directory_lister.h"

#include <atomic>
#include <chrono>
#include <sys/stat.h>
#include <unordered_map>

#include "async_p.h"
#include "base/strings.h"
#include "lingmo/files/file_enumerator.h"
#include "lingmo/files/file_system_url.h"
#include "framework_i18n.h"
#include "mime_sniffer_p.h"
#include "path_utils_p.h"
#include "trash_directory_p.h"

namespace Lingmo::Files {

using namespace std::chrono_literals;
using namespace Lingmo::Base;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

namespace {

// The MIME database as of a listing. The sniffer rereads it at most every five
// seconds, so types detected sooner after a change may be the old ones.
struct TypeDatabase
{
    std::string version;
    bool settled = false;
};

} // namespace

static int64_t modifiedMs(const std::string& path, bool* exists)
{
    struct stat st;
    *exists = ::stat(path.c_str(), &st) == 0;
    return *exists ? int64_t(st.st_mtim.tv_sec) * 1000 + st.st_mtim.tv_nsec / 1000000 : 0;
}

static TypeDatabase currentTypeDatabase()
{
    TypeDatabase database;
    int64_t newest = 0;

    for (const std::string& directory : internal::mimeDirectories()) {
        for (const std::string& path : {directory + "/mime.cache", directory + "/packages"}) {
            bool exists = false;
            const int64_t modified = modifiedMs(path, &exists);
            if (!exists)
                continue;
            newest = std::max(newest, modified);
            database.version += path + ' ' + strings::number((long long)modified) + '\n';
        }
    }

    const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    database.settled = now - newest > 6000;

    return database;
}

static bool isTrashRoot(const Url& url)
{
    return url.scheme() == "trash" && url.path() == "/";
}

static bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static std::string existingAncestor(std::string path)
{
    while (!isDirectory(path)) {
        const std::string parent = internal::absolutePathOf(path);
        if (parent == path)
            break;
        path = parent;
    }

    return path;
}

using PreviousByName = std::unordered_map<std::string, const FileItem*>;

// Stats every item and reads the files whose names do not decide their type,
// spread over helper threads for a first listing or a large folder.
static void prepareItems(FileItemList& items, const PreviousByName& previousByName,
                         const std::function<void(FileItem&)>& preparer, const OperationContext* context)
{
    constexpr ptrdiff_t BlockSize = 64;

    FileItem* const data = items.data();
    const ptrdiff_t count = ptrdiff_t(items.size());
    const ptrdiff_t blocks = (count + BlockSize - 1) / BlockSize;
    std::atomic<ptrdiff_t> nextBlock = 0;

    // A refresh keeps most types and only stats. Threads cut that from 220 to 40 ms
    // for 50,000 files, for 5-15% more CPU time per refresh; at 10,000 they saved nothing.
    const bool spread = previousByName.empty() || count >= internal::LargeFolder;

    internal::runOnFreeThreads(spread ? internal::blockHelpers(blocks) : 0, [&](const std::atomic_bool& stop) {
        const FileItem none;
        FileItem::TypeDetails details;

        for (ptrdiff_t block = nextBlock++; block < blocks && !stop; block = nextBlock++) {
            const ptrdiff_t end = std::min(count, (block + 1) * BlockSize);

            for (ptrdiff_t index = block * BlockSize; index < end; ++index) {
                context->checkpoint();
                FileItem& item = data[index];
                const auto found = previousByName.find(item.name());
                const FileItem* old = found == previousByName.cend() ? &none : found->second;
                if (!item.prepareMetadata(*old, &details) && preparer)
                    preparer(item);
            }
        }
    });
}

DirectoryLister::DirectoryLister()
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(40ms);
    m_retry.setInterval(2000ms);
    m_foundTimer.setInterval(100ms);

    m_watchChanged = m_watch.changed.connect([this] {
        if (m_autoUpdate)
            m_debounce.start();
    });
    m_watchError = m_watch.error.connect([this](const std::string& message) {
        reportError(message);
    });
    m_debounce.onTimeout = [this] {
        refresh();
    };
    m_retry.onTimeout = [this] {
        if (m_autoUpdate)
            refresh();
    };
    m_foundTimer.onTimeout = [this] {
        takeFound();
    };
}

DirectoryLister::~DirectoryLister()
{
    cancelListing();
}

void DirectoryLister::cancelListing()
{
    if (m_context)
        m_context->canceled.store(true);

    // Workers own their inputs and can outlive a view or navigation request.
    m_job.cancel();
    m_context.reset();
    m_pending = false;
    m_foundTimer.stop();
    m_found.reset();
}

void DirectoryLister::takeFound()
{
    FileItemList found;
    {
        const std::lock_guard lock(m_found->mutex);
        found.swap(m_found->items);
    }

    if (found.empty())
        return;

    m_items.insert(m_items.end(), found.begin(), found.end());
    itemsChanged.emit();
}

void DirectoryLister::openUrl(const Url& url)
{
    ++m_generation;
    cancelListing();
    m_debounce.stop();

    m_url = url;
    m_searchRoot = {};
    m_searchWords.clear();
    m_rootItem = {};
    m_items.clear();
    m_watch.clear();
    m_retry.stop();
    m_listed = false;
    m_lastError.clear();

    directoryChanged.emit();
    refresh();
}

void DirectoryLister::search(const Url& root, const std::string& text)
{
    const std::vector<std::string> words = strings::split(strings::simplified(text), ' ',
                                                          strings::SplitBehavior::SkipEmptyParts);
    if (words == m_searchWords && (words.empty() || root == m_searchRoot))
        return;

    ++m_generation;
    cancelListing();
    m_debounce.stop();

    m_searchRoot = words.empty() ? Url() : root;
    m_searchWords = words;
    m_rootItem = {};
    m_items.clear();
    m_watch.clear();
    m_retry.stop();
    m_listed = false;
    m_lastError.clear();

    directoryChanged.emit();
    refresh();
}

void DirectoryLister::updateDirectory(const Url& url)
{
    if (url == m_url)
        refresh();
}

void DirectoryLister::stop()
{
    ++m_generation;
    cancelListing();
    m_debounce.stop();
    m_retry.stop();
    m_watch.clear();
    canceled.emit();
}

void DirectoryLister::watchDirectory()
{
    // A watch on every folder searched would cost more than searching again.
    if (isSearching()) {
        m_watch.clear();
        m_retry.stop();
        return;
    }

    // Mounts and unmounts add and remove volumes' trash, so a folder listed can be
    // gone by now.
    if (isTrashRoot(m_url)) {
        std::vector<std::string> paths;
        for (const auto& path : m_trashPaths) {
            if (isDirectory(path))
                paths.push_back(path);
        }

        if (m_watch.watch(paths, m_trashEntries, true))
            m_retry.stop();
        else
            m_retry.start();
        return;
    }

    try {
        const std::string path = FileSystemUrl(m_url).localPath();

        // A path that is gone gets no watch and no polling; the listing either
        // moves to its parent or reports it once.
        if (!isDirectory(path)) {
            m_watch.clear();
            m_retry.stop();
        } else if (m_watch.watch(path)) {
            m_retry.stop();
        } else {
            m_retry.start();
        }
    } catch (const Failure&) {
        m_watch.clear();
        m_retry.stop();
    }
}

// Polling repeats the same failure every interval; show it once.
void DirectoryLister::reportError(const std::string& message)
{
    if (message == m_lastError)
        return;

    m_lastError = message;
    error.emit(message);
}

void DirectoryLister::refresh()
{
    if (m_url.isEmpty())
        return;

    if (m_job.isPending()) {
        m_pending = true;
        return;
    }

    // The trash's folders come with its listing; those of the last one can be gone.
    if (!isTrashRoot(m_url))
        watchDirectory();

    const uint64_t generation = m_generation;
    auto context = std::make_shared<OperationContext>();
    m_context = context;

    // Later searches of the same words replace the results at once instead of
    // emptying them first.
    std::shared_ptr<Found> found;
    if (isSearching() && !m_listed) {
        found = std::make_shared<Found>();
        m_found = found;
        m_foundTimer.start();
    }

    // A refresh of the listed directory keeps the detected types of unchanged files.
    auto work = [url = m_url, hidden = m_hidden, context, found, searchRoot = m_searchRoot, words = m_searchWords,
                 preparer = m_itemPreparer, listed = m_listed, previous = m_listed ? m_items : FileItemList(),
                 knownDatabase = m_typeDatabase, knownSettled = m_typesSettled]() mutable {
        Listing result;
        const TypeDatabase database = currentTypeDatabase();
        result.typeDatabase = database.version;
        result.typesSettled = database.settled;

        try {
            context->checkpoint();
            result.url = url;

            if (!words.empty()) {
                result.items = FileEnumerator::search(searchRoot, words, hidden, context.get(), [&](FileItem& item) {
                    if (preparer)
                        preparer(item);

                    if (found) {
                        const std::lock_guard lock(found->mutex);
                        found->items.push_back(item);
                    }
                });
                result.rootItem = FileItem(url);
                result.rootItem.prepareMetadata();
                previous = {};

                return result;
            }

            if (url.isLocalFile()) {
                const std::string target = internal::absoluteFilePath(url.toLocalFile());

                struct stat st;
                const bool present = ::stat(target.c_str(), &st) == 0;
                if (present && S_ISREG(st.st_mode))
                    result.url = Url::fromLocalFile(internal::absolutePathOf(target));
                else if (listed && !present)
                    result.url = Url::fromLocalFile(existingAncestor(target));
            }

            // Looking for the trash folders reads the mount table and stats each mount.
            if (isTrashRoot(result.url))
                internal::trashWatches(&result.trashPaths, &result.trashEntries);

            result.items = FileEnumerator::list(result.url, hidden, context.get());
            result.rootItem = FileItem(result.url);
            result.rootItem.prepareMetadata();

            static const FileItem none;
            PreviousByName previousByName;
            if (result.url == url && knownSettled && database.version == knownDatabase) {
                previousByName.reserve(previous.size());
                // Two files listed under one name cannot tell their entries apart.
                for (const auto& item : previous) {
                    const FileItem*& entry = previousByName[item.name()];
                    entry = entry ? &none : &item;
                }
            }

            prepareItems(result.items, previousByName, preparer, context.get());
        } catch (const Failure& failure) {
            result.error = failure.error;
        }

        // The model changes the old rows in place only while it holds them alone.
        previous = {};

        return result;
    };

    auto done = [this, generation](Listing listing) {
        if (generation != m_generation)
            return;

        m_context.reset();
        m_foundTimer.stop();
        m_found.reset();

        m_typeDatabase = listing.typeDatabase;
        m_typesSettled = listing.typesSettled;

        if (isTrashRoot(listing.url)) {
            m_trashPaths = listing.trashPaths;
            m_trashEntries = listing.trashEntries;
            watchDirectory();
        }

        if (listing.url.isValid() && !(listing.url == m_url)) {
            m_url = listing.url;
            watchDirectory();

            directoryChanged.emit();

            if (generation != m_generation)
                return;
        }

        // Not released in the background: the model frees the old rows, and changes
        // them in place only while it holds them alone.
        m_items = std::move(listing.items);
        m_rootItem = listing.rootItem;
        itemsChanged.emit();

        if (generation != m_generation)
            return;

        if (listing.error) {
            // TRANSLATORS: why a folder can't be listed, then the folder, such as "Permission denied: /root".
            reportError(format(tr("{error}: {path}"), {{"error", listing.error.message}, {"path", listing.error.path}}));
            if (generation == m_generation)
                canceled.emit();
        } else {
            m_listed = true;
            completed.emit();
        }

        if (generation == m_generation && m_pending) {
            m_pending = false;
            refresh();
        }
    };

    m_job = ThreadPool::global().run(std::move(work), std::move(done));

    started.emit();
}

} // namespace Lingmo::Files
