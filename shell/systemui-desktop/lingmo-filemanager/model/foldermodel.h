/***************************************************************************
 *   Copyright (C) 2006 David Faure <faure@kde.org>                        *
 *   Copyright (C) 2008 Fredrik Höglund <fredrik@kde.org>                  *
 *   Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>         *
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2021 Reven Martin <revenmartin@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include "../widgets/itemviewadapter.h"
#include "../helper/pathhistory.h"
#include "../mimetype/mimeappmanager.h"

#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QPointer>

#include <KDirLister>
#include <KDirModel>
#include <KDirWatch>
#include <KActionCollection>

class QDrag;
class CFileSizeJob;
class FolderModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QUrl resolvedUrl READ resolvedUrl NOTIFY resolvedUrlChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(bool sortDirsFirst READ sortDirsFirst WRITE setSortDirsFirst NOTIFY sortDirsFirstChanged)
    Q_PROPERTY(bool dragging READ dragging NOTIFY draggingChanged)
    Q_PROPERTY(QObject *viewAdapter READ viewAdapter WRITE setViewAdapter NOTIFY viewAdapterChanged)
    Q_PROPERTY(bool isDesktop READ isDesktop WRITE setIsDesktop NOTIFY isDesktopChanged)
    Q_PROPERTY(int selectionCount READ selectionCount NOTIFY selectionCountChanged)
    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)
    Q_PROPERTY(QString filterPattern READ filterPattern WRITE setFilterPattern NOTIFY filterPatternChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QStringList filterMimeTypes READ filterMimeTypes WRITE setFilterMimeTypes NOTIFY filterMimeTypesChanged)
    Q_PROPERTY(QString selectedItemSize READ selectedItemSize NOTIFY selectedItemSizeChanged)
    Q_PROPERTY(bool showHiddenFiles READ showHiddenFiles WRITE setShowHiddenFiles NOTIFY showHiddenFilesChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

public:
    enum DataRole {
        BlankRole = Qt::UserRole + 1,
        SelectedRole,
        IsDirRole,
        IsHiddenRole,
        IsLinkRole,
        UrlRole,
        DisplayNameRole,
        FileNameRole,
        FileSizeRole,
        IconNameRole,
        ThumbnailRole,
        ModifiedRole,
        IsDesktopFileRole
    };

    enum FilterMode {
        NoFilter = 0,
        FilterShowMatches,
        FilterHideMatches,
    };

    enum Status {
        None,
        Ready,
        Listing,
        Canceled,
    };
    Q_ENUM(Status)

    struct DragImage {
        int row;
        QRect rect;
        QPoint cursorOffset;
        QImage image;
        bool blank;
    };

    explicit FolderModel(QObject *parent = nullptr);
    ~FolderModel() override;

    void classBegin() override;
    void componentComplete() override;

    QHash<int, QByteArray> roleNames() const override;
    static QHash<int, QByteArray> staticRoleNames();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int indexForKeyboardSearch(const QString &text, int startFromIndex = 0) const;
    KFileItem itemForIndex(const QModelIndex &index) const;
    QModelIndex indexForUrl(const QUrl &url) const;

    KFileItem fileItem(int index) const;

    QList<QUrl> selectedUrls() const;

    int currentIndex() const;

    QString url() const;
    void setUrl(const QString &url);

    QUrl resolvedUrl() const;
    Q_INVOKABLE QUrl resolve(const QString &url);

    Status status() const;
    void setStatus(Status status);

    int sortMode() const;
    void setSortMode(int mode);

    bool sortDirsFirst() const;
    void setSortDirsFirst(bool enable);

    int filterMode() const;
    void setFilterMode(int filterMode);

    QStringList filterMimeTypes() const;
    void setFilterMimeTypes(const QStringList &mimeList);

    QString filterPattern() const;
    void setFilterPattern(const QString &pattern);

    QObject *viewAdapter() const;
    void setViewAdapter(QObject *adapter);

    bool dragging() const;

    bool isDir(const QModelIndex &index, const KDirModel *dirModel) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    KFileItem rootItem() const;

    int count() const;
    int selectionCount() const;

    Q_INVOKABLE QString homePath() const;
    Q_INVOKABLE QString desktopPath() const;
    Q_INVOKABLE QAction *action(const QString &name) const;

    Q_INVOKABLE void up();
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goForward();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void undo();

    Q_INVOKABLE bool supportSetAsWallpaper(const QString &mimeType);
    Q_INVOKABLE int fileExtensionBoundary(int row);

    Q_INVOKABLE bool hasSelection() const;
    Q_INVOKABLE bool isSelected(int row) const;
    Q_INVOKABLE bool isBlank(int row) const;
    Q_INVOKABLE void setSelected(int row);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void toggleSelected(int row);
    Q_INVOKABLE void setRangeSelected(int anchor, int to);
    Q_INVOKABLE void updateSelection(const QVariantList &rows, bool toggle);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void pinSelection();
    Q_INVOKABLE void unpinSelection();

    Q_INVOKABLE void newFolder();
    Q_INVOKABLE void newTextFile();
    Q_INVOKABLE void rename(int row, const QString &name);
    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();
    Q_INVOKABLE void cut();
    Q_INVOKABLE void openSelected();
    Q_INVOKABLE void showOpenWithDialog();
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void moveSelectedToTrash();
    Q_INVOKABLE void emptyTrash();
    Q_INVOKABLE void keyDeletePress();

    Q_INVOKABLE void setDragHotSpotScrollOffset(int x, int y);
    Q_INVOKABLE void addItemDragImage(int row, int x, int y, int width, int height, const QVariant &image);
    Q_INVOKABLE void clearDragImages();
    Q_INVOKABLE void dragSelected(int x, int y);
    Q_INVOKABLE void drop(QQuickItem *target, QObject *dropEvent, int row);

    Q_INVOKABLE void setWallpaperSelected();

    Q_INVOKABLE void openContextMenu(QQuickItem *visualParent = nullptr, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    Q_INVOKABLE void openPropertiesDialog();
    Q_INVOKABLE void openInTerminal();
    Q_INVOKABLE void openChangeWallpaperDialog();
    Q_INVOKABLE void openDeleteDialog();
    Q_INVOKABLE void openInNewWindow(const QString &url = QString());

    Q_INVOKABLE void updateSelectedItemsSize();
    Q_INVOKABLE void keyboardSearch(const QString &text);

    Q_INVOKABLE void clearPixmapCache();

    void restoreFromTrash();

    bool isDesktop() const;
    void setIsDesktop(bool isDesktop);

    QString selectedItemSize() const;

    bool showHiddenFiles() const;
    void setShowHiddenFiles(bool showHiddenFiles);

signals:
    void urlChanged();
    void listingCompleted() const;
    void listingCanceled() const;
    void resolvedUrlChanged();
    void statusChanged();
    void sortModeChanged();
    void sortDescChanged();
    void sortDirsFirstChanged();
    void filterModeChanged();
    void requestRename();
    void draggingChanged();
    void viewAdapterChanged();
    void isDesktopChanged();
    void selectionCountChanged();
    void countChanged();
    void filterPatternChanged();
    void filterMimeTypesChanged();
    void selectedItemSizeChanged();
    void showHiddenFilesChanged();
    void scrollToItem(int index);

    void notification(const QString &message);
    void move(int x, int y, QList<QUrl> urls);

    void currentIndexChanged();

private slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void dragSelectedInternal(int x, int y);
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void delayUpdateNeedSelectUrls();
    void updateNeedSelectUrls();

private:
    void invalidateIfComplete();
    void invalidateFilterIfComplete();
    void createActions();
    void updateActions();
    void addDragImage(QDrag *drag, int x, int y);

    bool isSupportThumbnails(const QString &mimeType) const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool matchMimeType(const KFileItem &item) const;
    bool matchPattern(const KFileItem &item) const;

private:
    KDirModel *m_dirModel;
    KDirWatch *m_dirWatch;
    KDirLister *m_dirLister;

    QItemSelectionModel *m_selectionModel;
    QItemSelection m_pinnedSelection;
    QString m_url;
    QUrl m_newDocumentUrl;
    QList<QUrl> m_needSelectUrls;

    Status m_status;
    int m_sortMode;
    bool m_sortDesc;
    bool m_sortDirsFirst;

    bool m_showHiddenFiles;

    FilterMode m_filterMode;
    QString m_filterPattern;
    bool m_filterPatternMatchAll;
    QSet<QString> m_mimeSet;
    QList<QRegExp> m_regExps;

    bool m_complete;
    bool m_isDesktop;
    bool m_suffixVisible;

    QString m_selectedItemSize;

    KActionCollection m_actionCollection;
    QHash<int, DragImage *> m_dragImages;
    QModelIndexList m_dragIndexes;
    QPoint m_dragHotSpotScrollOffset;
    bool m_dragInProgress;

    QHash<QString, QPoint> m_dropTargetPositions;
    QTimer *m_dropTargetPositionsCleanup;

    QPointer<ItemViewAdapter> m_viewAdapter;

    // Save path history
    PathHistory m_pathHistory;
    MimeAppManager *m_mimeAppManager;

    CFileSizeJob *m_sizeJob;

    int m_currentIndex;

    QTimer *m_updateNeedSelectTimer;
};

#endif // FOLDERMODEL_H
