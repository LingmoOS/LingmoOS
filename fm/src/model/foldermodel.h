// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "core/fileitem.h"
#include "core/lister.h"
#include "model/foldercontents.h"
#include "model/foldersizecache.h"
#include "model/pathhistory.h"
#include "ops/operations.h"

#include "base/signal.h"
#include "base/timer.h"

#include <cmath>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace fm {

// What a folder model asks of the windows around it: dialogs, other windows
// and messages. The browser and the desktop answer it each their own way.
class FolderModelHost
{
public:
    virtual ~FolderModelHost() = default;

    virtual void openFolders(const std::vector<std::string>& paths) = 0;
    virtual void showProperties(const FileItemList& items) = 0;
    virtual void showOpenWith(const std::string& url) = 0;
    virtual void confirmDelete(const std::vector<std::string>& urls) = 0;
    virtual void confirmEmptyTrash() = 0;
    virtual void confirmLaunch(const std::string& path) = 0;
    // A message in the window, for a few seconds.
    virtual void notify(const std::string& message) = 0;
};

// One folder as a view shows it: its listing sorted, the selection, the
// history, and what the menus do to its files.
class FolderModel
{
public:
    enum class Status {
        None,
        Ready,
        Listing,
        Canceled,
    };

    // A menu entry's state.
    struct Action
    {
        std::string text;
        bool visible = true;
        bool enabled = true;
    };

    explicit FolderModel(FolderModelHost* host = nullptr, bool desktop = false);
    ~FolderModel();

    FolderModel(const FolderModel&) = delete;
    FolderModel& operator=(const FolderModel&) = delete;

    void setHost(FolderModelHost* host) { m_host = host; }
    bool isDesktop() const { return m_isDesktop; }

    // Rows.
    int count() const { return m_contents.count(); }
    FileItemPtr item(int row) const { return m_contents.item(row); }
    int rowForUrl(const std::string& url) const { return m_contents.rowForUrl(url); }
    FolderContents& contents() { return m_contents; }
    const FileItem& rootItem() const { return m_lister.rootItem(); }

    // What the views show for a row.
    std::string displayName(int row) const;
    std::string fileSizeText(int row);
    std::string modifiedText(int row) const;
    std::string modifiedDateText(int row) const;
    std::string fileTypeText(int row) const;
    std::string originalPathText(int row) const;
    std::string deletedDateText(int row) const;
    // Whether the row shows a thumbnail instead of its type's icon.
    bool showsThumbnail(int row) const;
    // After the session language changed: the actions' text made again and
    // the rows sorted by the standard folders' new names. The views read
    // their text again themselves.
    void retranslate();

    // Location.
    const std::string& url() const { return m_url; }
    void setUrl(const std::string& url);
    std::string resolvedUrl() const { return isComputer() ? std::string("computer:///") : m_lister.url(); }
    bool isComputer() const { return m_url.starts_with("computer:"); }
    bool isTrash() const { return resolvedUrl().starts_with("trash:"); }
    // ~, ~/x, absolute paths and URLs, as the path bar takes them.
    static std::string resolve(const std::string& url);
    Status status() const { return m_status; }
    // The folder's name for its tab: a standard folder's shown name, Trash, or the last segment.
    std::string folderName() const;

    // Sorting.
    int sortMode() const { return m_sortMode; }
    void setSortMode(int mode);
    bool sortDesc() const { return m_sortDesc; }
    void setSortDesc(bool descending);
    void setSortDirsFirst(bool first)
    {
        m_sortDirsFirst = first;
        rebuildContents();
    }

    // History.
    bool canGoBack() const;
    bool canGoForward() const { return m_history.canGoForward(); }
    void up();
    void goBack();
    void goForward();
    void refresh();
    double viewPosition() const { return m_history.currentPosition(); }
    void setViewPosition(double position) { m_history.setCurrentPosition(position); }
    struct TabState
    {
        std::vector<std::string> back;
        std::vector<std::string> forward;
        std::vector<std::string> selection;
    };
    TabState tabState() const;
    void restoreHistory(const std::vector<std::string>& back, const std::vector<std::string>& forward);

    // Selection, by row.
    bool hasSelection() const { return !m_selection.empty(); }
    int selectionCount() const { return int(m_selection.size()); }
    bool isSelected(int row) const;
    void setSelected(int row);
    void toggleSelected(int row);
    void setRangeSelected(int anchor, int to);
    // Replaces the selection with the rows; toggle or extend apply them to the pinned one.
    void updateSelection(const std::vector<int>& rows, bool toggle, bool extend);
    void selectAll();
    void clearSelection();
    void pinSelection();
    void unpinSelection();
    std::vector<int> selectedRows() const;
    std::vector<std::string> selectedUrls() const;
    // The local path of the one selected item; empty for none or several.
    std::string selectedPath() const;
    int currentIndex() const { return m_currentIndex; }
    void selectUrlsWhenListed(const std::vector<std::string>& urls);
    void keyboardSearch(const std::string& text);
    // Where a rename's selection ends: before the extension.
    int fileExtensionBoundary(int row) const;

    // Search.
    const std::string& searchText() const { return m_searchText; }
    void setSearchText(const std::string& text);
    bool searchHome() const { return m_searchHome; }
    void setSearchHome(bool home);
    bool searchBusy() const { return m_searchBusy; }
    void showInFolder();
    bool isSearching() const { return m_lister.isSearching(); }

    bool showHiddenFiles() const { return m_showHiddenFiles; }
    void setShowHiddenFiles(bool show);

    // What the menus do.
    void prepareContextMenu();
    const Action& action(const std::string& name) const;
    void newFolder();
    void newTextFile();
    void rename(int row, const std::string& name);
    void copy();
    void cut();
    void paste();
    void pasteIntoSelection();
    void openSelected();
    void showOpenWithDialog();
    void deleteSelected();
    void moveSelectedToTrash();
    void emptyTrash();
    void keyDeletePress();
    void restoreFromTrash();
    void setWallpaperSelected();
    void openPropertiesDialog();
    void openInTerminal();
    void openChangeWallpaperDialog();
    void openDeleteDialog();
    void openInNewWindow(const std::string& url = {});
    void openInNewTab();
    void compressSelected();
    void extractSelectedArchive();
    void cancelArchive();
    void answerArchivePassword(uint32_t request, const std::string& password);
    void undo();
    void redo();
    bool openWithApplication(const std::string& desktopFile) const;
    static bool supportSetAsWallpaper(const std::string& mimeType);

    // The running archive job, for its password prompt.
    bool archiveBusy() const { return m_archive != nullptr; }
    bool archiveCancelling() const { return m_archiveCancelling; }
    std::string archiveSourceName() const { return m_archive ? m_archive->sourceName() : std::string(); }
    // The password request waiting for an answer; 0 when none is.
    uint32_t archivePasswordId() const { return m_archivePasswordId; }
    bool archivePasswordRetry() const { return m_archivePasswordRetry; }

    // Drops: the folder of row, or the folder shown for -1.
    std::string dropTargetUrl(int row) const;
    bool canDropUrls(const std::vector<std::string>& urls, int row, uint32_t action) const;
    // action is a ui::DropAction; x and y where it landed in the view, for the desktop.
    void drop(const std::vector<std::string>& urls, int row, uint32_t action, int x, int y);
    // A drag of the selection out of this view is running.
    bool dragging() const { return m_dragging; }
    void setDragging(bool dragging);
    // The rows a running drag carries, blank on the desktop while it lasts.
    bool isBlank(int row) const;

    // The desktop: where a new folder, file or drop should land.
    void setNewDocumentPosition(int x, int y, int cell = -1);
    int newDocumentCell(const std::string& name) const;
    void reserveCell(const std::string& name, int cell);

    base::Signal<> urlChanged;
    // A rename was asked for: the file shows under the second URL once done.
    base::Signal<std::string, std::string> renaming;
    // That rename failed or was cancelled: the file keeps its first URL.
    base::Signal<std::string, std::string> renameFailed;
    base::Signal<> statusChanged;
    base::Signal<> historyChanged;
    base::Signal<> selectionChanged;
    base::Signal<> countChanged;
    // Rows changed in content or order; views lay out and bind again.
    base::Signal<const ContentsChange&> rowsChanged;
    // A row's data changed (a folder's item count arrived).
    base::Signal<int> rowDataChanged;
    base::Signal<> sortChanged;
    base::Signal<> searchChanged;
    base::Signal<> showHiddenFilesChanged;
    base::Signal<> archiveChanged;
    base::Signal<> listingCompleted;
    base::Signal<> aboutToLeaveFolder;
    base::Signal<> requestRename;
    base::Signal<int> scrollToItem;
    base::Signal<std::string> openTabRequested;
    base::Signal<std::string> notification;
    base::Signal<> actionsChanged;
    base::Signal<> draggingChanged;
    // The desktop: files dropped inside it move to where they were dropped.
    base::Signal<int, int, std::vector<std::string>> move;
    // The desktop: a file arrived where a drop or new document wanted it.
    base::Signal<int, int, std::vector<std::string>> placeArrived;

private:
    void setStatus(Status status);
    void rebuildContents();
    void updateSearch();
    void setSearchBusy(bool busy);
    void leaveFolder();
    void selectWhenListed(const std::vector<std::string>& urls);
    void applyPendingSelection();
    void contentsChanged(const ContentsChange& change);
    void updateNeedSelectUrls();
    void rememberNewDocumentPosition(const std::string& name);
    void startArchive(OperationTaskPtr task, const std::string& operation, const std::string& outputPath);
    void archiveFinished(bool success, bool canceled, const std::string& outputPath);
    // Mounts the image through lingmo-mountd; its volume shows in the sidebar.
    void mountDiskImage(const std::string& path);
    void pasteTo(const std::string& destination, const std::string& selectIn);
    void createActions();
    void updateActions();
    std::string rootUrl() const;
    void select(int row);
    void deselect(int row);
    void syncSelectionRows();

    FolderModelHost* m_host;
    const bool m_isDesktop;
    DirectoryLister m_lister;
    FolderContents m_contents;
    FolderSizeCache m_folderSizes;
    PathHistory m_history;
    std::string m_url;
    Status m_status = Status::None;
    int m_sortMode = 0;
    bool m_sortDesc = false;
    bool m_sortDirsFirst = true;
    bool m_showHiddenFiles = false;
    std::string m_searchText;
    bool m_searchHome = true;
    bool m_searchBusy = false;

    // Selected files by URL, which survives reordering; the rows follow it.
    std::set<std::string> m_selection;
    std::set<std::string> m_pinned;
    std::vector<bool> m_selectedRows;
    int m_currentIndex = -1;
    std::vector<std::string> m_pendingSelection;
    std::vector<std::string> m_needSelectUrls;
    base::Timer m_needSelectTimer;
    std::string m_newDocumentUrl;

    // The desktop.
    int m_newDocumentX = -1;
    int m_newDocumentY = -1;
    int m_newDocumentCell = -1;
    std::map<std::string, std::pair<int, int>> m_dropTargetPositions;
    std::map<std::string, int> m_newDocumentCells;
    base::Timer m_dropTargetPositionsCleanup;
    bool m_dragging = false;
    std::set<std::string> m_dragUrls;

    std::map<std::string, Action> m_actions;
    std::vector<std::string> m_clipboardTypes;

    OperationTaskPtr m_archive;
    bool m_archiveCancelling = false;
    std::string m_archiveOperation;
    std::string m_archiveOutputPath;
    uint32_t m_archivePasswordId = 0;
    bool m_archivePasswordRetry = false;
    base::Subscription m_archiveChanged;
    base::Subscription m_archiveFinished;

    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
