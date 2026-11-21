// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/foldermodel.h"
#include "core/config.h"
#include "core/dates.h"
#include "core/launcher.h"
#include "core/mounts.h"
#include "core/preferences.h"
#include "core/standardfolders.h"
#include "core/thumbnails.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/mime_database.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"
#include "base/url.h"
#include "ui/clipboard.h"
#include "ui/drag.h"

#include "lingmo/files/url.h"

#include <algorithm>
#include <fcntl.h>
#include <functional>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trc;
using Lingmo::i18n::trn;

namespace fm {

static constexpr char kCutSelection[] = "application/x-lingmo-cutselection";

static const std::vector<std::string> kArchiveMimeTypes = {
    "application/gzip",
    "application/vnd.rar",
    "application/x-7z-compressed",
    "application/x-bzip",
    "application/x-bzip-compressed-tar",
    "application/x-bzip2",
    "application/x-bzip2-compressed-tar",
    "application/x-compressed-tar",
    "application/x-lzma",
    "application/x-lzma-compressed-tar",
    "application/x-rar",
    "application/x-tar",
    "application/x-xz",
    "application/x-xz-compressed-tar",
    "application/x-zip-compressed",
    "application/x-zstd-compressed-tar",
    "application/zip",
    "application/zstd",
};

static const std::vector<std::string> kFolderArchiveSuffixes = {
    ".tar.gz", ".tgz", ".tar.bz2", ".tbz2", ".tbz", ".tar.xz", ".txz",
    ".tar.zst", ".tzst", ".tar.lzma", ".tar", ".zip", ".7z", ".rar",
};

static const std::vector<std::string> kSingleFileSuffixes = {".gz", ".bz2", ".xz", ".zst", ".lzma"};

// The suffix extraction drops: "a.tar.gz" becomes the folder "a", while
// "a.txt.gz" becomes the file "a.txt". Every part of a multi-part archive
// ("a.part2.rar", "a.7z.002") extracts the whole of it to "a".
static std::string archiveSuffix(const std::string& name, bool* singleFile)
{
    using base::strings::CaseSensitivity;
    static const std::regex part(R"((\.part\d+\.rar|\.(7z|zip)\.\d{3,})$)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(name, match, part)) {
        *singleFile = false;
        return match.str(1);
    }
    for (const std::string& suffix : kFolderArchiveSuffixes) {
        if (base::strings::endsWith(name, suffix, CaseSensitivity::Insensitive)) {
            *singleFile = false;
            return suffix;
        }
    }
    for (const std::string& suffix : kSingleFileSuffixes) {
        if (base::strings::endsWith(name, suffix, CaseSensitivity::Insensitive)) {
            *singleFile = true;
            return suffix;
        }
    }
    *singleFile = false;
    return {};
}

static bool isArchive(const FileItem& item)
{
    if (!item.isLocalFile() || item.isDir)
        return false;
    if (std::find(kArchiveMimeTypes.begin(), kArchiveMimeTypes.end(), item.mimeType) != kArchiveMimeTypes.end())
        return true;
    bool singleFile = false;
    return !archiveSuffix(item.name, &singleFile).empty();
}

static bool exists(const std::string& path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0;
}

static std::string uniquePath(const std::string& directory, const std::string& name, bool directoryPath)
{
    std::string stem = name;
    std::string suffix;
    if (!directoryPath) {
        const size_t dot = name.rfind('.');
        if (dot != std::string::npos && dot > 0 && dot + 1 < name.size()) {
            suffix = name.substr(dot);
            stem = name.substr(0, dot);
        }
    }

    std::string candidate = directory + '/' + name;
    for (int index = 1; exists(candidate); ++index)
        candidate = directory + '/' + stem + ' ' + std::to_string(index) + suffix;
    return candidate;
}

// The folder a URL is in; empty past the root.
static std::string upUrl(const std::string& url)
{
    const files::Url up = files::upUrl(files::Url(url));
    return up.isValid() && !up.isEmpty() ? up.toString() : std::string();
}

static std::string localPathOf(const std::string& url)
{
    return DirectoryLister::localPathForUrl(url);
}

// Every URL is directly in the folder: a drop between two views of it.
static bool isDropBetweenSharedViews(const std::vector<std::string>& urls, const std::string& folderUrl)
{
    const std::string folder = localPathOf(folderUrl);
    for (const std::string& url : urls) {
        if (parentPath(localPathOf(url)) != folder)
            return false;
    }
    return true;
}

static std::string hiddenSettingsPath(bool desktop)
{
    (void)desktop;
    return settingsFile();
}

static std::string hiddenSettingsKey(bool desktop)
{
    return desktop ? "Desktop/showHiddenFiles" : "showHiddenFiles";
}

FolderModel::FolderModel(FolderModelHost* host, bool desktop)
    : m_host(host)
    , m_isDesktop(desktop)
{
    {
        base::Settings settings(hiddenSettingsPath(m_isDesktop));
        m_showHiddenFiles = settings.boolValue(hiddenSettingsKey(m_isDesktop), false);
    }

    m_needSelectTimer.setSingleShot(true);
    m_needSelectTimer.setInterval(std::chrono::milliseconds(50));
    m_needSelectTimer.onTimeout = [this] {
        updateNeedSelectUrls();
    };

    m_dropTargetPositionsCleanup.setSingleShot(true);
    m_dropTargetPositionsCleanup.setInterval(std::chrono::seconds(10));
    m_dropTargetPositionsCleanup.onTimeout = [this] {
        m_dropTargetPositions.clear();
        m_newDocumentCells.clear();
    };

    m_lister.setAutoUpdate(true);
    m_lister.setShowHiddenFiles(m_showHiddenFiles);
    m_contents.setLister(&m_lister);

    m_subscriptions.push_back(m_lister.itemsChanged.connect([this] {
        m_folderSizes.clear();
    }));
    m_subscriptions.push_back(m_folderSizes.ready.connect([this](const std::string& path) {
        int row = m_contents.rowForUrl(base::url::fromLocalFile(path));
        // Items of the trash carry trash:/ URLs, so only their local path matches.
        if (row < 0)
            row = m_contents.rowForLocalPath(path);
        if (row >= 0)
            rowDataChanged.emit(row);
    }));
    m_subscriptions.push_back(m_lister.error.connect([this](const std::string& message) {
        notification.emit(message);
    }));
    m_subscriptions.push_back(m_lister.started.connect([this] {
        setStatus(Status::Listing);
        setSearchBusy(m_lister.isSearching());
    }));
    m_subscriptions.push_back(m_lister.canceled.connect([this] {
        setStatus(Status::Canceled);
        setSearchBusy(false);
    }));
    m_subscriptions.push_back(m_lister.completed.connect([this] {
        setSearchBusy(false);
        if (!m_contents.isPreparing() && m_lister.items().empty() && m_contents.count() == 0) {
            setStatus(Status::Ready);
            listingCompleted.emit();
        }
    }));
    // Results are not watched; whatever a file operation changed shows once it ends.
    m_subscriptions.push_back(OperationsClient::instance().activeTaskCountChanged.connect([this](int count) {
        if (count == 0 && m_lister.isSearching())
            m_contents.schedulePreparation();
    }));

    m_subscriptions.push_back(m_contents.preparationStarted.connect([this] {
        setStatus(Status::Listing);
    }));
    m_subscriptions.push_back(m_contents.rowsChanged.connect([this](const ContentsChange& change) {
        contentsChanged(change);
    }));
    m_subscriptions.push_back(m_contents.preparationFinished.connect([this] {
        setStatus(Status::Ready);
        applyPendingSelection();
        listingCompleted.emit();
        updateActions();
    }));
    m_subscriptions.push_back(m_contents.reset.connect([this] {
        m_currentIndex = -1;
        syncSelectionRows();
        countChanged.emit();
        rowsChanged.emit(ContentsChange {{}, {}, {}, true});
    }));

    createActions();
    m_subscriptions.push_back(ui::Clipboard::changed().connect([this](ui::ClipboardMode mode) {
        if (mode == ui::ClipboardMode::Clipboard)
            updateActions();
    }));
    m_subscriptions.push_back(OperationsClient::instance().historyChanged.connect([this] {
        updateActions();
    }));

    rebuildContents();
}

FolderModel::~FolderModel()
{
    *m_alive = false;
}

void FolderModel::rebuildContents()
{
    m_contents.setQuery({m_sortMode, m_sortDesc, m_sortDirsFirst});
}

void FolderModel::setStatus(Status status)
{
    if (m_status == status)
        return;
    m_status = status;
    statusChanged.emit();
}

void FolderModel::contentsChanged(const ContentsChange& change)
{
    // New files placed where a drop or a new document wanted them.
    if (!m_dropTargetPositions.empty() && m_sortMode >= 0 && !change.added.empty())
        setSortMode(-1);

    syncSelectionRows();
    if (change.reordered) {
        const auto rows = selectedRows();
        m_currentIndex = rows.empty() ? -1 : rows.front();
    }
    countChanged.emit();
    rowsChanged.emit(change);

    if (change.added.empty())
        return;

    // A drop or a new document said where these go.
    std::string renameUrl;
    for (const std::string& url : change.added) {
        const std::string name = fileName(localPathOf(url));
        m_newDocumentCells.erase(name);
        if (const auto found = m_dropTargetPositions.find(name); found != m_dropTargetPositions.end()) {
            const auto [x, y] = found->second;
            m_dropTargetPositions.erase(found);
            // Later: the views are still taking the new rows in.
            base::EventLoop::current()->post([this, alive = m_alive, x, y, url] {
                if (*alive)
                    move.emit(x, y, {url});
            });
        }
        if (url == m_newDocumentUrl) {
            renameUrl = url;
            m_newDocumentUrl.clear();
        }
    }

    // A new folder is selected before its rename starts.
    if (!renameUrl.empty()) {
        auto timer = std::make_shared<base::Timer>();
        timer->setSingleShot(true);
        timer->onTimeout = [this, alive = m_alive, renameUrl, timer]() mutable {
            if (*alive) {
                const int row = m_contents.rowForUrl(renameUrl);
                if (row >= 0) {
                    setSelected(row);
                    requestRename.emit();
                }
            }
            base::EventLoop::current()->post([timer] {});
            timer.reset();
        };
        timer->start(std::chrono::milliseconds(60));
    }

    m_needSelectTimer.start();
}

std::string FolderModel::displayName(int row) const
{
    const FileItemPtr file = item(row);
    if (!file)
        return {};
    if (file->isDir && file->isLocalFile()) {
        const std::string standard = standardFolderName(file->path);
        if (!standard.empty())
            return standard;
    }
    return file->text();
}

std::string FolderModel::fileSizeText(int row)
{
    const FileItemPtr file = item(row);
    if (!file)
        return {};
    if (file->isDir) {
        const int64_t count = m_folderSizes.count(file->path);
        if (count < 0)
            return {};
        return trn("{n} item", "{n} items", uint64_t(count));
    }
    return formatSize(file->size);
}

std::string FolderModel::modifiedText(int row) const
{
    const FileItemPtr file = item(row);
    return file ? dates::friendlyTime(file->modified) : std::string();
}

std::string FolderModel::modifiedDateText(int row) const
{
    const FileItemPtr file = item(row);
    return file ? dates::columnTime(file->modified) : std::string();
}

std::string FolderModel::fileTypeText(int row) const
{
    const FileItemPtr file = item(row);
    if (!file)
        return {};
    if (file->isDir)
        return tr("Folder");
    return file->typeDescription.empty() ? file->mimeType : file->typeDescription;
}

std::string FolderModel::originalPathText(int row) const
{
    const FileItemPtr file = item(row);
    return file ? FolderContents::originalLocation(*file) : std::string();
}

std::string FolderModel::deletedDateText(int row) const
{
    const FileItemPtr file = item(row);
    return file ? dates::columnTime(file->deletionTime) : std::string();
}

bool FolderModel::showsThumbnail(int row) const
{
    const FileItemPtr file = item(row);
    // Thumbnails of a phone's or a server's files would read each one whole.
    if (!file || !file->isLocalFile() || file->isDir || thumbnails::isOnMount(file->path))
        return false;
    return thumbnails::canThumbnail(file->mimeType);
}

void FolderModel::retranslate()
{
    forgetStandardFolderNames();
    // Unsorted rows keep their order whatever the names.
    if (m_sortMode >= 0)
        m_contents.retakeSnapshots();
    createActions();
    updateActions();
}

std::string FolderModel::resolve(const std::string& url)
{
    if (url == "~")
        return base::url::fromLocalFile(homePath());
    if (url.starts_with("~/"))
        return base::url::fromLocalFile(cleanPath(homePath() + '/' + url.substr(2)));
    if (url.starts_with('/'))
        return base::url::fromLocalFile(cleanPath(url));

    const size_t colon = url.find(':');
    if (colon != std::string::npos && colon > 0) {
        bool scheme = true;
        for (size_t i = 0; i < colon; ++i) {
            const char c = url[i];
            if (!base::strings::isAsciiAlpha(c) && !base::strings::isAsciiDigit(c) && c != '+' && c != '-'
                && c != '.')
                scheme = false;
        }
        if (scheme)
            return url;
    }

    char current[4096];
    const std::string directory = ::getcwd(current, sizeof(current)) ? current : homePath();
    return base::url::fromLocalFile(cleanPath(directory + '/' + url));
}

void FolderModel::setUrl(const std::string& url)
{
    if (url.empty())
        return;

    const bool trash = url.starts_with("trash:/");
    const bool computer = url == "computer:///";
    const std::string resolved = trash ? std::string("trash:/") : computer ? std::string("computer:///") : resolve(url);

    // Opening a folder ends a search; opening the searched one lists it again.
    const bool wasSearching = !m_searchText.empty();
    if (wasSearching) {
        m_searchText.clear();
        searchChanged.emit();
        if (resolved == resolvedUrl()) {
            updateSearch();
            historyChanged.emit();
            return;
        }
    }

    if (resolved == resolvedUrl()) {
        if (computer)
            setStatus(Status::Ready);
        else
            refresh();
        historyChanged.emit();
        return;
    }

    setStatus(Status::Listing);
    m_pendingSelection.clear();

    if (m_history.isEmpty() || m_history.last() != resolved) {
        // The results' scroll offset is not the folder's.
        if (!wasSearching)
            leaveFolder();
        m_history.append(resolved);
    }

    const auto local = base::url::toLocalFile(resolved);
    m_url = local ? *local : resolved;
    m_dragUrls.clear();
    m_selection.clear();
    m_contents.clear();
    m_folderSizes.reset();
    if (computer) {
        setStatus(Status::Ready);
    } else {
        m_lister.openUrl(resolved);
    }

    urlChanged.emit();
    historyChanged.emit();
    selectionChanged.emit();
}

std::string FolderModel::folderName() const
{
    if (isComputer())
        return tr("Computer");
    if (m_url.starts_with("trash:/"))
        return tr("Trash");
    std::string path = m_url;
    while (path.size() > 1 && path.back() == '/')
        path.pop_back();
    const std::string standard = standardFolderName(path);
    if (!standard.empty())
        return standard;
    const std::string name = fileName(path);
    return name.empty() ? std::string("/") : name;
}

void FolderModel::setSortMode(int mode)
{
    if (mode < -1 || mode > Preferences::DeletionTime || m_sortMode == mode)
        return;
    m_sortMode = mode;
    rebuildContents();
    sortChanged.emit();
}

void FolderModel::setSortDesc(bool descending)
{
    if (m_sortDesc == descending)
        return;
    m_sortDesc = descending;
    rebuildContents();
    sortChanged.emit();
}

bool FolderModel::canGoBack() const
{
    return !m_searchText.empty() || m_history.canGoBack();
}

void FolderModel::up()
{
    const std::string child = resolvedUrl();
    const std::string parent = upUrl(child);
    if (parent.empty())
        return;
    setUrl(parent);
    selectWhenListed({child});
}

void FolderModel::goBack()
{
    if (!m_searchText.empty()) {
        setSearchText({});
        return;
    }
    if (!m_history.canGoBack())
        return;

    leaveFolder();
    setUrl(m_history.previousPath());
    selectWhenListed(m_history.currentSelection());
}

void FolderModel::goForward()
{
    if (!m_history.canGoForward())
        return;

    if (m_searchText.empty())
        leaveFolder();
    setUrl(m_history.posteriorPath());
    selectWhenListed(m_history.currentSelection());
}

void FolderModel::leaveFolder()
{
    aboutToLeaveFolder.emit();
    m_history.setCurrentSelection(selectedUrls());
}

void FolderModel::refresh()
{
    m_lister.refresh();
}

FolderModel::TabState FolderModel::tabState() const
{
    return {m_history.backward(), m_history.forward(), selectedUrls()};
}

void FolderModel::restoreHistory(const std::vector<std::string>& back, const std::vector<std::string>& forward)
{
    m_history.restore(back, forward);
    historyChanged.emit();
}

void FolderModel::syncSelectionRows()
{
    m_selectedRows.assign(size_t(count()), false);
    std::set<std::string> kept;
    for (const std::string& url : m_selection) {
        const int row = m_contents.rowForUrl(url);
        if (row >= 0) {
            m_selectedRows[size_t(row)] = true;
            kept.insert(url);
        }
    }
    if (kept.size() != m_selection.size()) {
        m_selection = std::move(kept);
        selectionChanged.emit();
        updateActions();
    }
    if (m_currentIndex >= count())
        m_currentIndex = -1;
}

bool FolderModel::isSelected(int row) const
{
    return row >= 0 && size_t(row) < m_selectedRows.size() && m_selectedRows[size_t(row)];
}

void FolderModel::select(int row)
{
    if (const FileItemPtr file = item(row)) {
        m_selection.insert(file->url);
        if (size_t(row) < m_selectedRows.size())
            m_selectedRows[size_t(row)] = true;
    }
}

void FolderModel::deselect(int row)
{
    if (const FileItemPtr file = item(row)) {
        m_selection.erase(file->url);
        if (size_t(row) < m_selectedRows.size())
            m_selectedRows[size_t(row)] = false;
    }
}

void FolderModel::setSelected(int row)
{
    if (row < 0)
        return;
    const bool was = isSelected(row);
    select(row);
    m_currentIndex = row;
    if (!was) {
        selectionChanged.emit();
        updateActions();
    }
}

void FolderModel::toggleSelected(int row)
{
    if (row < 0)
        return;
    if (isSelected(row))
        deselect(row);
    else
        select(row);
    selectionChanged.emit();
    updateActions();
}

void FolderModel::setRangeSelected(int anchor, int to)
{
    if (anchor < 0 || to < 0)
        return;
    m_selection.clear();
    std::fill(m_selectedRows.begin(), m_selectedRows.end(), false);
    for (int row = std::min(anchor, to); row <= std::max(anchor, to); ++row)
        select(row);
    selectionChanged.emit();
    updateActions();
}

void FolderModel::updateSelection(const std::vector<int>& rows, bool toggle, bool extend)
{
    std::set<std::string> next = (toggle || extend) ? m_pinned : std::set<std::string>();
    for (const int row : rows) {
        if (row < 0)
            return;
        const FileItemPtr file = item(row);
        if (!file)
            continue;
        if (toggle && next.contains(file->url))
            next.erase(file->url);
        else
            next.insert(file->url);
    }
    if (next == m_selection)
        return;
    m_selection = std::move(next);
    m_selectedRows.assign(size_t(count()), false);
    for (const std::string& url : m_selection) {
        const int row = m_contents.rowForUrl(url);
        if (row >= 0)
            m_selectedRows[size_t(row)] = true;
    }
    selectionChanged.emit();
    updateActions();
}

void FolderModel::selectAll()
{
    setRangeSelected(0, count() - 1);
}

void FolderModel::clearSelection()
{
    if (m_selection.empty())
        return;
    m_selection.clear();
    std::fill(m_selectedRows.begin(), m_selectedRows.end(), false);
    selectionChanged.emit();
    updateActions();
}

void FolderModel::pinSelection()
{
    m_pinned = m_selection;
}

void FolderModel::unpinSelection()
{
    m_pinned.clear();
}

std::vector<int> FolderModel::selectedRows() const
{
    std::vector<int> rows;
    for (size_t row = 0; row < m_selectedRows.size(); ++row) {
        if (m_selectedRows[row])
            rows.push_back(int(row));
    }
    return rows;
}

std::vector<std::string> FolderModel::selectedUrls() const
{
    std::vector<std::string> urls;
    for (const int row : selectedRows())
        urls.push_back(item(row)->url);
    return urls;
}

std::string FolderModel::selectedPath() const
{
    const auto rows = selectedRows();
    return rows.size() == 1 ? item(rows.front())->path : std::string();
}

void FolderModel::selectUrlsWhenListed(const std::vector<std::string>& urls)
{
    std::vector<std::string> resolved;
    for (const std::string& url : urls)
        resolved.push_back(resolve(url));
    selectWhenListed(resolved);
}

void FolderModel::selectWhenListed(const std::vector<std::string>& urls)
{
    m_pendingSelection = urls;
    if (m_status == Status::Ready)
        applyPendingSelection();
}

void FolderModel::applyPendingSelection()
{
    if (m_pendingSelection.empty())
        return;

    const std::vector<std::string> urls = std::exchange(m_pendingSelection, {});
    int first = -1;
    for (const std::string& url : urls) {
        const int row = m_contents.rowForUrl(url);
        if (row < 0)
            continue;
        if (first < 0) {
            clearSelection();
            first = row;
        }
        setSelected(row);
    }
    if (first >= 0)
        scrollToItem.emit(first);
}

void FolderModel::updateNeedSelectUrls()
{
    std::vector<int> rows;
    for (const std::string& url : m_needSelectUrls) {
        const int row = m_contents.rowForUrl(url);
        if (row >= 0)
            rows.push_back(row);
    }
    m_needSelectUrls.clear();
    if (rows.empty())
        return;
    clearSelection();
    for (const int row : rows)
        setSelected(row);
}

void FolderModel::keyboardSearch(const std::string& text)
{
    if (count() == 0)
        return;

    const auto rows = selectedRows();
    const int current = rows.empty() ? -1 : rows.front();
    const int start = (current + 1) % count();
    const auto matches = [&](int row) {
        return base::strings::startsWith(item(row)->text(), text, base::strings::CaseSensitivity::Insensitive);
    };

    int found = -1;
    for (int row = start; row < count() && found < 0; ++row) {
        if (matches(row))
            found = row;
    }
    for (int row = 0; row < start && found < 0; ++row) {
        if (matches(row))
            found = row;
    }
    if (found < 0 || found == current)
        return;

    clearSelection();
    setSelected(found);
    scrollToItem.emit(found);
}

int FolderModel::fileExtensionBoundary(int row) const
{
    const FileItemPtr file = item(row);
    if (!file)
        return 0;
    const std::string& name = file->name;
    if (file->isDir)
        return int(name.size());

    // The extension the type's glob names, else the last dot's.
    const std::string type = base::mime::typeForFileName(name);
    std::string suffix;
    for (const std::string& known : kFolderArchiveSuffixes) {
        if (name.ends_with(known)) {
            suffix = known.substr(1);
            break;
        }
    }
    if (!suffix.empty() && type != "application/octet-stream")
        return int(name.size() - suffix.size() - 1);

    const size_t dot = name.rfind('.');
    return dot == std::string::npos || dot < 1 ? int(name.size()) : int(dot);
}

void FolderModel::setSearchText(const std::string& text)
{
    if (m_searchText == text)
        return;
    m_searchText = text;
    updateSearch();
    searchChanged.emit();
    historyChanged.emit();
}

void FolderModel::setSearchHome(bool home)
{
    if (m_searchHome == home)
        return;
    m_searchHome = home;
    updateSearch();
    searchChanged.emit();
}

void FolderModel::setSearchBusy(bool busy)
{
    if (m_searchBusy == busy)
        return;
    m_searchBusy = busy;
    searchChanged.emit();
}

void FolderModel::updateSearch()
{
    m_pendingSelection.clear();
    m_dragUrls.clear();
    const std::string root = m_searchHome ? base::url::fromLocalFile(homePath()) : resolvedUrl();
    m_selection.clear();
    m_contents.clear();
    if (m_searchText.empty())
        m_lister.openUrl(resolvedUrl());
    else
        m_lister.search(root, m_searchText);

    selectionChanged.emit();
}

void FolderModel::showInFolder()
{
    const auto rows = selectedRows();
    if (rows.size() != 1)
        return;
    const std::string url = item(rows.front())->url;
    setUrl(upUrl(url));
    selectWhenListed({url});
}

void FolderModel::setShowHiddenFiles(bool show)
{
    if (m_showHiddenFiles == show)
        return;
    m_showHiddenFiles = show;
    m_lister.setShowHiddenFiles(show);
    m_lister.refresh();

    base::Settings settings(hiddenSettingsPath(m_isDesktop));
    settings.setValue(hiddenSettingsKey(m_isDesktop), show);
    settings.sync();

    showHiddenFilesChanged.emit();
    updateActions();
}

std::string FolderModel::rootUrl() const
{
    const FileItem& root = rootItem();
    return root.url.empty() ? resolvedUrl() : root.url;
}

void FolderModel::rememberNewDocumentPosition(const std::string& name)
{
    if (m_newDocumentX < 0 || m_newDocumentY < 0)
        return;
    m_dropTargetPositions[name] = {m_newDocumentX, m_newDocumentY};
    if (m_newDocumentCell >= 0)
        m_newDocumentCells[name] = m_newDocumentCell;
    m_dropTargetPositionsCleanup.start();
    m_newDocumentX = -1;
    m_newDocumentY = -1;
    m_newDocumentCell = -1;
}

void FolderModel::setNewDocumentPosition(int x, int y, int cell)
{
    m_newDocumentX = x;
    m_newDocumentY = y;
    m_newDocumentCell = cell;
}

int FolderModel::newDocumentCell(const std::string& name) const
{
    const auto found = m_newDocumentCells.find(name);
    return found != m_newDocumentCells.end() ? found->second : -1;
}

void FolderModel::reserveCell(const std::string& name, int cell)
{
    if (cell < 0)
        return;
    m_newDocumentCells[name] = cell;
    m_dropTargetPositionsCleanup.start();
}

void FolderModel::newFolder()
{
    const std::string root = rootItem().path;
    if (root.empty())
        return;

    const std::string base = trc("default file name", "New Folder");
    std::string name = base;
    for (int i = 1; exists(root + '/' + name); ++i)
        name = base + std::to_string(i);

    const std::string url = base::url::fromLocalFile(root + '/' + name);
    m_newDocumentUrl = url;
    rememberNewDocumentPosition(name);
    OperationsClient::instance().mkdir(url);
}

void FolderModel::newTextFile()
{
    const std::string root = rootItem().path;
    if (root.empty())
        return;

    const std::string base = trc("default file name", "New Text");
    std::string name = base;
    for (int i = 1; exists(root + '/' + name); ++i)
        name = base + std::to_string(i);

    m_newDocumentUrl = base::url::fromLocalFile(root + '/' + name);
    rememberNewDocumentPosition(name);

    const int fd = ::open((root + '/' + name).c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd >= 0) {
        (void)!::write(fd, "\n", 1);
        ::close(fd);
    }
}

// Calls settle once the task ends, unless alive is set and false by then.
static void whenFinished(const OperationTaskPtr& task, const std::shared_ptr<bool>& alive,
                         std::function<void(const OperationTask&)> settle)
{
    if (task->isFinished()) {
        settle(*task);
        return;
    }

    std::weak_ptr<OperationTask> weak = task;
    auto subscription = std::make_shared<base::Subscription>();
    *subscription = task->finished.connect([alive, weak, subscription, settle = std::move(settle)] {
        if (const OperationTaskPtr finished = weak.lock(); finished && (!alive || *alive))
            settle(*finished);
        // The subscription goes with the next loop turn, not inside its own call.
        base::EventLoop::current()->post([subscription] { subscription->reset(); });
    });
}

// A cut stays on the clipboard until its move is done: after a move that
// failed or was cancelled, it holds what is still in its place, so pasting
// again moves the rest. Nothing changes once something else was put there.
static void settleCut(const std::string& uriList, const std::vector<std::string>& urls, const OperationTask& move)
{
    std::vector<std::string> remaining;
    if (move.error() || move.wasCanceled()) {
        struct stat st;
        for (const std::string& url : urls) {
            const std::string path = localPathOf(url);
            if (path.empty() || ::lstat(path.c_str(), &st) == 0)
                remaining.push_back(url);
        }
    }
    if (remaining.size() == urls.size())
        return;

    const auto types = ui::Clipboard::mimeTypes();
    if (std::find(types.begin(), types.end(), kCutSelection) == types.end())
        return;
    ui::Clipboard::requestData("text/uri-list", [uriList, remaining](std::optional<std::string> data) {
        if (!data || *data != uriList)
            return;
        if (remaining.empty()) {
            ui::Clipboard::clear();
            return;
        }
        ui::MimeData mime;
        mime.setUrls(remaining);
        mime.setData(kCutSelection, "1");
        ui::Clipboard::setMimeData(mime);
    });
}

void FolderModel::rename(int row, const std::string& name)
{
    if (row < 0 || name.empty() || name == "." || name == ".." || name.find('/') != std::string::npos
        || name.find('\0') != std::string::npos)
        return;

    const FileItemPtr file = item(row);
    if (!file || !file->isLocalFile())
        return;

    const std::string destination = base::url::fromLocalFile(parentPath(file->path) + '/' + name);
    m_contents.expectRename(file->url, destination);
    renaming.emit(file->url, destination);
    const OperationTaskPtr task = OperationsClient::instance().rename(file->url, destination);
    whenFinished(task, m_alive, [this, from = file->url, destination](const OperationTask& outcome) {
        if (outcome.error() || outcome.wasCanceled()) {
            m_contents.forgetRename(from, destination);
            renameFailed.emit(from, destination);
        }
    });
}

void FolderModel::copy()
{
    if (m_selection.empty() || !action("copy").enabled)
        return;

    ui::MimeData data;
    data.setUrls(selectedUrls());
    ui::Clipboard::setMimeData(data);
}

void FolderModel::cut()
{
    if (m_selection.empty() || !action("cut").enabled)
        return;

    ui::MimeData data;
    data.setUrls(selectedUrls());
    data.setData(kCutSelection, "1");
    ui::Clipboard::setMimeData(data);
}

void FolderModel::paste()
{
    if (rootItem().path.empty() || !rootItem().isWritable)
        return;

    pasteTo(resolvedUrl(), rootItem().path);
}

void FolderModel::pasteIntoSelection()
{
    const auto rows = selectedRows();
    if (rows.size() != 1 || isTrash())
        return;

    const FileItemPtr folder = item(rows.front());
    if (!folder->isDir || folder->path.empty() || !folder->isWritable)
        return;

    pasteTo(folder->url, {});
}

void FolderModel::pasteTo(const std::string& destination, const std::string& selectIn)
{
    const auto types = ui::Clipboard::mimeTypes();
    const bool hasUrls = std::find(types.begin(), types.end(), "text/uri-list") != types.end();
    if (!hasUrls)
        return;

    const bool cut = std::find(types.begin(), types.end(), kCutSelection) != types.end();
    ui::Clipboard::requestData("text/uri-list", [this, alive = m_alive, cut, destination,
                                                 selectIn](std::optional<std::string> data) {
        if (!*alive || !data)
            return;
        ui::MimeData mime;
        mime.setData("text/uri-list", *data);
        const std::vector<std::string> urls = mime.urls();
        if (urls.empty())
            return;
        if (!selectIn.empty()) {
            for (const std::string& url : urls)
                m_needSelectUrls.push_back(base::url::fromLocalFile(selectIn + '/' + fileName(localPathOf(url))));
        }
        if (!cut) {
            OperationsClient::instance().copy(urls, destination);
            return;
        }
        // The clipboard is settled even after this view has gone.
        whenFinished(OperationsClient::instance().move(urls, destination), nullptr,
                     [uriList = *data, urls](const OperationTask& move) { settleCut(uriList, urls, move); });
    });
}

void FolderModel::openSelected()
{
    if (m_selection.empty() || isTrash())
        return;

    const auto rows = selectedRows();
    if (!m_isDesktop && rows.size() == 1 && item(rows.front())->isDir) {
        setUrl(item(rows.front())->url);
        return;
    }

    if (rows.size() == 1 && isArchive(*item(rows.front()))) {
        extractSelectedArchive();
        return;
    }

    for (const int row : rows) {
        const FileItemPtr file = item(row);
        if (m_isDesktop && file->isDir) {
            openInNewWindow(file->url);
            continue;
        }

        const std::string& mimeType = file->mimeType;
        if (mimeType == "application/x-apple-diskimage" && file->isLocalFile() && !file->isDir) {
            mountDiskImage(file->path);
            continue;
        }

        // Installed or executable launchers run; a local launcher that is not
        // executable yet asks first, then stays executable and trusted.
        if (mimeType == "application/x-desktop") {
            if (launcher::isTrusted(file->path))
                launcher::launchFile(file->path);
            else if (m_host)
                m_host->confirmLaunch(file->path);
            continue;
        }

        // A program's mode bits and owner come with the medium it is on, so every one asks first.
        if (mimeType == "application/x-executable" || mimeType == "application/x-sharedlib"
            || mimeType == "application/x-iso9660-appimage" || mimeType == "application/vnd.appimage") {
            if (m_host)
                m_host->confirmLaunch(file->path);
            continue;
        }

        std::string application = launcher::defaultApplication(mimeType);
        // Without a default, the first of the applications for the type.
        if (application.empty()) {
            const auto recommended = launcher::recommendedApplications(mimeType);
            if (!recommended.empty())
                application = recommended.front();
        }
        if (!application.empty()) {
            launcher::launchApplication(application, file->path);
            continue;
        }

        launcher::startCommand({"xdg-open", file->url});
    }
}

void FolderModel::mountDiskImage(const std::string& path)
{
    MountsClient::instance().requestMount(
            path, {}, [this, alive = m_alive](const std::string& status, const base::dbus::VariantMap& details) {
                if (!*alive || status == "mounted")
                    return;
                const auto found = details.find("message");
                notification.emit(found != details.end() ? found->second.value<std::string>() : std::string());
            });
}

void FolderModel::showOpenWithDialog()
{
    const auto urls = selectedUrls();
    if (!urls.empty() && m_host)
        m_host->showOpenWith(urls.front());
}

bool FolderModel::openWithApplication(const std::string& desktopFile) const
{
    const auto rows = selectedRows();
    if (rows.size() != 1 || desktopFile.empty())
        return false;
    const FileItemPtr file = item(rows.front());
    return launcher::launchApplication(desktopFile, file->isLocalFile() ? file->path : file->url);
}

void FolderModel::deleteSelected()
{
    if (m_selection.empty() || !action("del").enabled)
        return;
    OperationsClient::instance().remove(selectedUrls());
}

void FolderModel::moveSelectedToTrash()
{
    if (m_selection.empty() || !action("trash").enabled)
        return;
    OperationsClient::instance().trash(selectedUrls());
}

void FolderModel::emptyTrash()
{
    if (m_host)
        m_host->confirmEmptyTrash();
}

void FolderModel::keyDeletePress()
{
    if (m_selection.empty())
        return;
    if (isTrash())
        openDeleteDialog();
    else
        moveSelectedToTrash();
}

void FolderModel::restoreFromTrash()
{
    if (m_selection.empty() || !action("restore").visible)
        return;
    OperationsClient::instance().restore(selectedUrls());
}

bool FolderModel::supportSetAsWallpaper(const std::string& mimeType)
{
    return mimeType == "image/jpeg" || mimeType == "image/png";
}

void FolderModel::setWallpaperSelected()
{
    const auto rows = selectedRows();
    if (rows.empty())
        return;
    const FileItemPtr file = item(rows.front());
    if (!file->isLocalFile())
        return;

    base::dbus::Proxy appearance(base::dbus::Connection::session(), "org.lingmo.Services",
                                 "/org/lingmo/Services/Appearance", "org.lingmo.Services.Appearance");
    appearance.send("setWallpaper", file->path);
}

void FolderModel::openPropertiesDialog()
{
    if (!m_host)
        return;
    FileItemList items;
    const auto rows = selectedRows();
    if (rows.empty()) {
        items.push_back(std::make_shared<const FileItem>(rootItem()));
    } else {
        for (const int row : rows)
            items.push_back(item(row));
    }
    m_host->showProperties(items);
}

void FolderModel::openInTerminal()
{
    std::string path;
    const auto rows = selectedRows();
    if (!rows.empty()) {
        if (item(rows.front())->isDir)
            path = item(rows.front())->path;
    } else {
        path = rootItem().path;
    }
    launcher::launchTerminal(path);
}

void FolderModel::openChangeWallpaperDialog()
{
    launcher::startCommand({"lingmo-settings", "-m", "background"});
}

void FolderModel::openDeleteDialog()
{
    if (m_host && !m_selection.empty())
        m_host->confirmDelete(selectedUrls());
}

void FolderModel::openInNewWindow(const std::string& url)
{
    if (!m_host)
        return;
    if (!url.empty()) {
        // Callers pass either a local path or a folder URL.
        const auto local = base::url::toLocalFile(url);
        m_host->openFolders({local ? *local : url});
        return;
    }

    // Without a URL, every selected folder.
    std::vector<std::string> folders;
    for (const int row : selectedRows()) {
        if (item(row)->isDir)
            folders.push_back(item(row)->path);
    }
    m_host->openFolders(folders);
}

void FolderModel::openInNewTab()
{
    for (const int row : selectedRows()) {
        if (item(row)->isDir)
            openTabRequested.emit(item(row)->url);
    }
}

void FolderModel::undo()
{
    if (OperationsClient::instance().undoAvailable())
        OperationsClient::instance().undo();
}

void FolderModel::redo()
{
    if (OperationsClient::instance().redoAvailable())
        OperationsClient::instance().redo();
}

void FolderModel::startArchive(OperationTaskPtr task, const std::string& operation, const std::string& outputPath)
{
    if (!task)
        return;

    m_archive = task;
    m_archiveCancelling = false;
    m_archiveOperation = operation;
    m_archiveOutputPath = outputPath;
    m_archivePasswordId = 0;
    m_archivePasswordRetry = false;
    archiveChanged.emit();

    m_archiveChanged = task->changed.connect([this] {
        if (!m_archive)
            return;
        const uint32_t passwordId = m_archive->waitingForPassword() ? m_archive->passwordId() : 0;
        const bool passwordRetry = m_archive->passwordRetry();
        if (passwordId != m_archivePasswordId || passwordRetry != m_archivePasswordRetry) {
            m_archivePasswordId = passwordId;
            m_archivePasswordRetry = passwordRetry;
            archiveChanged.emit();
        }
    });
    m_archiveFinished = task->finished.connect([this] {
        const OperationTaskPtr task = m_archive;
        if (!task)
            return;
        archiveFinished(!task->error() && !task->wasCanceled(), task->wasCanceled(), m_archiveOutputPath);
    });
}

void FolderModel::compressSelected()
{
    if (m_archive || m_selection.empty() || isTrash() || !rootItem().isWritable)
        return;

    std::vector<std::string> paths;
    for (const int row : selectedRows()) {
        const FileItemPtr file = item(row);
        if (!file->isLocalFile() || file->path.empty()) {
            notification.emit(tr("Only local files can be compressed."));
            return;
        }
        paths.push_back(file->path);
    }

    const std::string destination = rootItem().path;
    if (destination.empty())
        return;

    const std::string baseName
            = paths.size() == 1 ? fileName(paths.front()) : std::string(trc("default file name", "Archive"));
    const std::string outputPath = uniquePath(destination, baseName + ".zip", false);
    startArchive(OperationsClient::instance().archive(0, paths, outputPath), "Compressing", outputPath);
}

void FolderModel::extractSelectedArchive()
{
    const auto rows = selectedRows();
    if (m_archive || rows.size() != 1)
        return;

    const FileItemPtr file = item(rows.front());
    if (!isArchive(*file) || !file->isFile)
        return;

    bool singleFile = false;
    const std::string suffix = archiveSuffix(file->name, &singleFile);
    std::string baseName;
    if (suffix.empty()) {
        // The base name runs up to the last dot.
        const size_t dot = file->name.rfind('.');
        baseName = dot == std::string::npos ? file->name : file->name.substr(0, dot);
    } else {
        baseName = file->name.substr(0, file->name.size() - suffix.size());
    }
    if (baseName.empty())
        baseName = trc("default file name", "Extracted");

    const std::string outputPath = uniquePath(parentPath(file->path), baseName, !singleFile);
    startArchive(OperationsClient::instance().archive(1, {file->path}, outputPath), "Extracting", outputPath);
}

void FolderModel::cancelArchive()
{
    if (!m_archive || m_archiveCancelling)
        return;
    m_archiveCancelling = true;
    archiveChanged.emit();
    m_archive->cancel();
}

void FolderModel::answerArchivePassword(uint32_t request, const std::string& password)
{
    if (m_archive && request != 0 && request == m_archivePasswordId)
        m_archive->answerPassword(request, password);
}

void FolderModel::archiveFinished(bool success, bool canceled, const std::string& outputPath)
{
    const std::string operation = m_archiveOperation;
    m_archive.reset();
    m_archiveChanged.reset();
    m_archiveCancelling = false;
    m_archivePasswordId = 0;
    m_archivePasswordRetry = false;
    archiveChanged.emit();
    updateActions();

    if (success) {
        m_needSelectUrls.push_back(base::url::fromLocalFile(outputPath));
        refresh();
        auto timer = std::make_shared<base::Timer>();
        timer->setSingleShot(true);
        timer->onTimeout = [this, alive = m_alive, timer]() mutable {
            if (*alive)
                updateNeedSelectUrls();
            base::EventLoop::current()->post([timer] {});
            timer.reset();
        };
        timer->start(std::chrono::milliseconds(100));
    } else if (canceled) {
        notification.emit(operation == "Compressing" ? tr("Compressing canceled.") : tr("Extracting canceled."));
    }
    // A failure is shown by the task's own dialog.
}

std::string FolderModel::dropTargetUrl(int row) const
{
    if (row >= 0) {
        const FileItemPtr file = item(row);
        if (!file || !file->isDir)
            return {};
        return file->path.empty() ? file->url : base::url::fromLocalFile(file->path);
    }
    if (m_lister.isSearching())
        return {};
    const FileItem& root = rootItem();
    if (!root.path.empty())
        return base::url::fromLocalFile(root.path);
    return resolvedUrl();
}

bool FolderModel::canDropUrls(const std::vector<std::string>& urls, int row, uint32_t action) const
{
    if (urls.empty() || action == ui::NoAction)
        return false;

    const std::string targetUrl = dropTargetUrl(row);
    if (targetUrl.empty())
        return false;

    const std::string targetPath = cleanPath(localPathOf(targetUrl));
    if (!targetUrl.starts_with("trash:")) {
        struct stat st;
        if (targetPath.empty() || ::stat(targetPath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)
            || ::access(targetPath.c_str(), W_OK | X_OK) != 0)
            return false;
    }

    if (m_isDesktop && m_dragging && row == -1 && isDropBetweenSharedViews(urls, targetUrl))
        return true;

    const bool moving = action == ui::MoveAction;
    for (const std::string& url : urls) {
        const std::string sourcePath = cleanPath(localPathOf(url));
        if (url == targetUrl || sourcePath == targetPath)
            return false;

        if (!sourcePath.empty() && !targetPath.empty() && base::url::isLocalFile(url)) {
            if (parentPath(sourcePath) == targetPath || targetPath.starts_with(sourcePath + '/'))
                return false;
            if (moving && ::access(parentPath(sourcePath).c_str(), W_OK | X_OK) != 0)
                return false;
        }

        if (moving && !canMoveStandardFolderTo(sourcePath, targetPath))
            return false;
    }
    return true;
}

void FolderModel::drop(const std::vector<std::string>& urls, int row, uint32_t action, int x, int y)
{
    // Whatever cell the view set for this drop is used up, whether it is used or not.
    const int cell = m_newDocumentCell;
    m_newDocumentX = -1;
    m_newDocumentY = -1;
    m_newDocumentCell = -1;

    if (!canDropUrls(urls, row, action))
        return;

    const std::string targetUrl = dropTargetUrl(row);

    if (m_dragging && row == -1) {
        // The files are here already: no rows come for them, so a position kept
        // for their names would only move some later file of the same name.
        setSortMode(-1);
        move.emit(x, y, urls);
        return;
    }

    if (!isDropBetweenSharedViews(urls, targetUrl)) {
        if (m_isDesktop && row == -1) {
            for (const std::string& url : urls)
                m_dropTargetPositions[fileName(localPathOf(url))] = {x, y};
            // The first file takes the cell itself; the rest spread out from it.
            reserveCell(fileName(localPathOf(urls.front())), cell);
            m_dropTargetPositionsCleanup.start();
        }

        if (action == ui::MoveAction)
            OperationsClient::instance().move(urls, targetUrl);
        else if (action == ui::CopyAction)
            OperationsClient::instance().copy(urls, targetUrl);
        else
            OperationsClient::instance().link(urls, targetUrl);

        for (const std::string& url : urls)
            m_needSelectUrls.push_back(base::url::fromLocalFile(rootItem().path + '/' + fileName(localPathOf(url))));
    }
}

void FolderModel::setDragging(bool dragging)
{
    if (m_dragging == dragging)
        return;
    m_dragging = dragging;
    if (dragging)
        m_dragUrls = m_selection;
    else
        m_dragUrls.clear();
    draggingChanged.emit();
}

bool FolderModel::isBlank(int row) const
{
    if (row < 0)
        return true;
    if (!m_isDesktop || m_dragUrls.empty())
        return false;
    const FileItemPtr file = item(row);
    return file && m_dragUrls.contains(file->url);
}

void FolderModel::prepareContextMenu()
{
    m_clipboardTypes = ui::Clipboard::mimeTypes();
    updateActions();
}

const FolderModel::Action& FolderModel::action(const std::string& name) const
{
    static const Action none {{}, false, false};
    const auto found = m_actions.find(name);
    return found != m_actions.end() ? found->second : none;
}

void FolderModel::createActions()
{
    const std::vector<std::pair<std::string, std::string>> actions = {
        {"open", tr("Open")},
        {"openWith", tr("Open with")},
        {"showInFolder", tr("Show in Enclosing Folder")},
        {"compress", tr("Compress")},
        {"cut", tr("Cut")},
        {"copy", tr("Copy")},
        {"paste", tr("Paste")},
        {"pasteInto", tr("Paste")},
        {"newFolder", tr("New Folder")},
        {"newTextFile", tr("New Text")},
        {"trash", tr("Move To Trash")},
        {"emptyTrash", tr("Empty Trash")},
        {"del", tr("Delete")},
        {"rename", tr("Rename")},
        {"terminal", tr("Open in Terminal")},
        {"wallpaper", tr("Set as Wallpaper")},
        {"properties", tr("Properties")},
        {"changeBackground", tr("Change background")},
        {"restore", tr("Restore")},
        {"showHidden", tr("Show Hidden Files")},
        {"openInNewWindow", tr("Open In New Window")},
        {"openInNewTab", tr("Open In New Tab")},
        {"undo", tr("Undo")},
        {"redo", tr("Redo")},
    };
    for (const auto& [name, text] : actions)
        m_actions[name] = Action {text, true, true};
}

void FolderModel::updateActions()
{
    const auto rows = selectedRows();
    FileItemList items;
    bool hasRemoteFiles = false;
    bool hasDir = false;
    bool hasStandardFolder = false;
    const bool trash = isTrash();

    if (rows.empty()) {
        items.push_back(std::make_shared<const FileItem>(rootItem()));
    } else {
        for (const int row : rows) {
            const FileItemPtr file = item(row);
            hasRemoteFiles |= file->path.empty();
            items.push_back(file);
            if (file->isDir) {
                hasDir = true;
                hasStandardFolder |= file->isLocalFile() && isStandardFolder(file->path);
            }
        }
    }

    const bool searching = m_lister.isSearching();
    // Search results come from many folders; each must let its file go.
    const bool canMoveItems = !items.empty()
            && (searching ? std::all_of(items.begin(), items.end(),
                                        [](const FileItemPtr& file) {
                                            return ::access(parentPath(file->path).c_str(), W_OK) == 0;
                                        })
                          : rootItem().isWritable);

    const auto set = [this](const std::string& name, bool visible, bool enabled) {
        Action& entry = m_actions[name];
        entry.visible = visible;
        entry.enabled = enabled;
    };

    set("open", !trash && !hasDir, true);
    // The context menu only lists these with a selection; the desktop menu bar always.
    set("copy", !trash, !rows.empty());
    set("cut", !trash, !rows.empty() && canMoveItems && !hasStandardFolder);
    set("restore", items.size() >= 1 && trash, true);
    set("openWith", items.size() == 1 && !trash && !hasDir, true);
    set("showInFolder", searching && rows.size() == 1, true);

    const bool canCompress = !rows.empty() && !trash && !hasRemoteFiles && !searching && rootItem().isWritable;
    set("compress", canCompress, canCompress && !m_archive);
    m_actions["compress"].text = rows.size() == 1 && !items.empty()
                                         ? format(tr("Compress “{name}”"), {{"name", items.front()->name}})
                                         : std::string(tr("Compress Items"));

    set("newFolder", !trash && !searching, rootItem().isWritable);
    set("newTextFile", !trash && !searching, rootItem().isWritable);

    if (m_clipboardTypes.empty())
        m_clipboardTypes = ui::Clipboard::mimeTypes();
    const bool clipboardUrls
            = std::find(m_clipboardTypes.begin(), m_clipboardTypes.end(), "text/uri-list") != m_clipboardTypes.end();
    set("paste", !trash && !searching, clipboardUrls && !rootItem().path.empty() && rootItem().isWritable);
    const bool singleFolder = rows.size() == 1 && hasDir;
    set("pasteInto", singleFolder && !trash && !hasRemoteFiles,
        singleFolder && clipboardUrls && items.front()->isWritable);
    m_clipboardTypes.clear();

    set("rename", !trash && !(items.front()->isLocalFile() && isStandardFolder(items.front()->path)), canMoveItems);
    set("trash", !trash, canMoveItems && !hasStandardFolder);
    set("emptyTrash", trash && count() > 0, true);
    set("del", trash && canMoveItems, true);
    set("terminal", items.size() == 1 && items.front()->isDir && !trash, true);
    set("wallpaper", items.size() == 1 && !trash && supportSetAsWallpaper(items.front()->mimeType), true);
    set("properties", !trash, true);
    set("showHidden", !trash, true);
    // Plain text instead of a check mark: the label says what the item does.
    m_actions["showHidden"].text = m_showHiddenFiles ? tr("Hide Hidden Files") : tr("Show Hidden Files");
    set("openInNewWindow", hasDir && !trash, true);
    set("openInNewTab", hasDir && !trash && !m_isDesktop, true);
    set("undo", true, OperationsClient::instance().undoAvailable());
    set("redo", true, OperationsClient::instance().redoAvailable());

    actionsChanged.emit();
}

} // namespace fm
