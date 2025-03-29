/***************************************************************************
 *   Copyright (C) 2006 David Faure <faure@kde.org>                        *
 *   Copyright (C) 2008 Fredrik Höglund <fredrik@kde.org>                  *
 *   Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>         *
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2021 Reven Martin <revenmartin@gmail.com>               *
 *   Copyright (C) 2021 Reion Wong <reionwong@gmail.com>                   *
 *   Copyright (C) 2025 Lingmo OS Team <team@lingmo.org>                   *
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

#include "foldermodel.h"
#include "dirlister.h"
#include "window.h"

#include "../dialogs/filepropertiesdialog.h"
#include "../dialogs/createfolderdialog.h"
#include "../dialogs/openwithdialog.h"
#include "../dialogs/deletedialog.h"

#include "../helper/datehelper.h"
#include "../helper/filelauncher.h"
#include "../helper/fm.h"

#include "../cio/cfilesizejob.h"

// Qt
#include <QSet>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QCollator>
#include <QDBusInterface>
#include <QStandardPaths>
#include <QApplication>
#include <QDesktopWidget>
#include <QMimeDatabase>
#include <QMimeData>
#include <QClipboard>
#include <QPainter>
#include <QDrag>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QDesktopServices>
#include <QPixmapCache>
#include <QFileDialog>

// Qt Quick
#include <QQuickItem>
#include <QQmlContext>

// KIO
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KIO/PreviewJob>
#include <KIO/DeleteJob>
#include <KIO/DropJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KIO/RestoreJob>
#include <KUrlMimeData>
#include <KFileItemListProperties>
#include <KDesktopFile>

// KService
#include <KServiceTypeTrader>
#include <KRun>


static bool isDropBetweenSharedViews(const QList<QUrl> &urls, const QUrl &folderUrl)
{
    for (const auto &url : urls) {
        if (folderUrl.adjusted(QUrl::StripTrailingSlash) != url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash)) {
            return false;
        }
    }
    return true;
}

FolderModel::FolderModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_dirWatch(nullptr)
    , m_status(None)
    , m_sortMode(0)
    , m_sortDesc(false)
    , m_sortDirsFirst(true)
    , m_showHiddenFiles(false)
    , m_filterMode(NoFilter)
    , m_filterPatternMatchAll(true)
    , m_complete(false)
    , m_isDesktop(false)
    , m_selectedItemSize("")
    , m_actionCollection(this)
    , m_dragInProgress(false)
    , m_dropTargetPositionsCleanup(new QTimer(this))
    , m_viewAdapter(nullptr)
    , m_mimeAppManager(MimeAppManager::self())
    , m_sizeJob(nullptr)
    , m_currentIndex(-1)
    , m_updateNeedSelectTimer(new QTimer(this))
{
    QSettings settings("lingmoos", qApp->applicationName());
    m_showHiddenFiles = settings.value("showHiddenFiles", false).toBool();

    m_updateNeedSelectTimer->setSingleShot(true);
    m_updateNeedSelectTimer->setInterval(50);
    connect(m_updateNeedSelectTimer, &QTimer::timeout, this, &FolderModel::updateNeedSelectUrls);

    m_dirLister = new DirLister(this);
    m_dirLister->setDelayedMimeTypes(true);
    m_dirLister->setAutoErrorHandlingEnabled(false, nullptr);
    m_dirLister->setAutoUpdate(true);
    m_dirLister->setShowingDotFiles(m_showHiddenFiles);
    // connect(dirLister, &DirLister::error, this, &FolderModel::notification);

    connect(m_dirLister, &KCoreDirLister::started, this, std::bind(&FolderModel::setStatus, this, Status::Listing));

    void (KCoreDirLister::*myCompletedSignal)() = &KCoreDirLister::completed;
    QObject::connect(m_dirLister, myCompletedSignal, this, [this] {
        setStatus(Status::Ready);
        emit listingCompleted();
    });

    void (KCoreDirLister::*myCanceledSignal)() = &KCoreDirLister::canceled;
    QObject::connect(m_dirLister, myCanceledSignal, this, [this] {
        setStatus(Status::Canceled);
        emit listingCanceled();
    });

    m_dirModel = new KDirModel(this);
    m_dirModel->setDirLister(m_dirLister);
    m_dirModel->setDropsAllowed(KDirModel::DropOnDirectory | KDirModel::DropOnLocalExecutable);
    m_dirModel->moveToThread(qApp->thread());

    // If we have dropped items queued for moving, go unsorted now.
    connect(this, &QAbstractItemModel::rowsAboutToBeInserted, this, [this]() {
        if (!m_dropTargetPositions.isEmpty()) {
            setSortMode(-1);
        }
    });

    // Position dropped items at the desired target position.
    connect(this, &QAbstractItemModel::rowsInserted, this, &FolderModel::onRowsInserted);

    /*
     * Dropped files may not actually show up as new files, e.g. when we overwrite
     * an existing file. Or files that fail to be listed by the dirLister, or...
     * To ensure we don't grow the map indefinitely, clean it up periodically.
     * The cleanup timer is (re)started whenever we modify the map. We use a quite
     * high interval of 10s. This should ensure, that we don't accidentally wipe
     * the mapping when we actually still want to use it. Since the time between
     * adding an entry in the map and it showing up in the model should be
     * small, this should rarely, if ever happen.
     */
    m_dropTargetPositionsCleanup->setInterval(10000);
    m_dropTargetPositionsCleanup->setSingleShot(true);
    connect(m_dropTargetPositionsCleanup, &QTimer::timeout, this, [this]() {
        if (!m_dropTargetPositions.isEmpty()) {
            qDebug() << "clearing drop target positions after timeout:" << m_dropTargetPositions;
            m_dropTargetPositions.clear();
        }
    });

    m_selectionModel = new QItemSelectionModel(this, this);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &FolderModel::selectionChanged);

    setSourceModel(m_dirModel);
    setSortLocaleAware(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);

    sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
    createActions();

    connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

FolderModel::~FolderModel()
{

}

void FolderModel::classBegin()
{

}

void FolderModel::componentComplete()
{
    m_complete = true;
    invalidate();
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    return staticRoleNames();
}

QHash<int, QByteArray> FolderModel::staticRoleNames()
{
    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    roleNames[BlankRole] = "blank";
    roleNames[SelectedRole] = "selected";
    roleNames[IsDirRole] = "isDir";
    roleNames[IsHiddenRole] = "isHidden";
    roleNames[IsLinkRole] = "isLink";
    roleNames[UrlRole] = "url";
    roleNames[DisplayNameRole] = "displayName";
    roleNames[FileNameRole] = "fileName";
    roleNames[FileSizeRole] = "fileSize";
    roleNames[IconNameRole] = "iconName";
    roleNames[ThumbnailRole] = "thumbnail";
    roleNames[ModifiedRole] = "modified";
    roleNames[IsDesktopFileRole] = "desktopFile";
    return roleNames;
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    KFileItem item = itemForIndex(index);

    switch (role) {
    case BlankRole:
        return m_dragIndexes.contains(index);
    case SelectedRole:
        return m_selectionModel->isSelected(index);
    case UrlRole:
        return item.url();
    case DisplayNameRole: {
        if (item.isDesktopFile()) {
            KDesktopFile dfile(item.localPath());

            if (!dfile.readName().isEmpty())
                return dfile.readName();
        }

        return item.url().fileName();
    }
    case FileNameRole: {
        return item.url().fileName();
    }
    case IsDesktopFileRole: {
        return item.isDesktopFile();
    }
    case IsDirRole: {
        return item.isDir();
    }
    case IsHiddenRole: {
        return item.isHidden();
    }
    case IsLinkRole: {
        return item.isLink();
    }
    case FileSizeRole: {
        if (item.isDir()) {
            QDir dir(item.url().toLocalFile());
            dir.setFilter(QDir::Dirs | QDir::AllEntries | QDir::NoDotAndDotDot);
            uint count = dir.count();
            return count == 1 ? tr("%1 item").arg(count) : tr("%1 items").arg(count);
        }

        return KIO::convertSize(item.size());
    }
    case IconNameRole:
        return item.iconName();
    case ThumbnailRole: {
        if (item.isLocalFile()) {
            // Svg Image
            if (item.mimetype() == "image/svg" ||
                    item.mimetype() == "image/svg+xml") {
                return item.url();
            }

            // Support
            if (isSupportThumbnails(item.mimetype())) {
                return "image://thumbnailer/" + item.url().toString();
            }
        }

        return QVariant();
    }
    case ModifiedRole: {
        return DateHelper::friendlyTime(item.time(KFileItem::ModificationTime));
    }
    default:
        break;
    }

    return QSortFilterProxyModel::data(index, role);
}

int FolderModel::indexForKeyboardSearch(const QString &text, int startFromIndex) const
{
    startFromIndex = qMax(0, startFromIndex);

    for (int i = startFromIndex; i < rowCount(); ++i) {
        if (fileItem(i).text().startsWith(text, Qt::CaseInsensitive)) {
            return i;
        }
    }

    for (int i = 0; i < startFromIndex; ++i) {
        if (fileItem(i).text().startsWith(text, Qt::CaseInsensitive)) {
            return i;
        }
    }

    return -1;
}

KFileItem FolderModel::itemForIndex(const QModelIndex &index) const
{
    return m_dirModel->itemForIndex(mapToSource(index));
}

QModelIndex FolderModel::indexForUrl(const QUrl &url) const
{
    return m_dirModel->indexForUrl(url);
}

KFileItem FolderModel::fileItem(int index) const
{
    if (index >= 0 && index < count()) {
        return itemForIndex(FolderModel::index(index, 0));
    }

    return KFileItem();
}

QList<QUrl> FolderModel::selectedUrls() const
{
    const auto indexes = m_selectionModel->selectedIndexes();

    QList<QUrl> urls;
    urls.reserve(indexes.count());

    for (const QModelIndex &index : indexes) {
        urls.append(itemForIndex(index).url());
    }

    return urls;
}

int FolderModel::currentIndex() const
{
    return m_currentIndex;
}

QString FolderModel::url() const
{
    return m_url;
}

void FolderModel::setUrl(const QString &url)
{
    if (url.isEmpty())
        return;

    bool isTrash = url.startsWith("trash:/");
    QUrl resolvedNewUrl = resolve(url);
    QFileInfo info(resolvedNewUrl.toLocalFile());

    if (!QFile::exists(resolvedNewUrl.toLocalFile()) && !isTrash) {
        emit notification(tr("The file or folder %1 does not exist.").arg(url));
        return;
    }

    // TODO: selected ?
    if (info.isFile() && !isTrash) {
        resolvedNewUrl = QUrl::fromLocalFile(info.dir().path());
    }

    // Refresh this directory.
    if (url == m_url) {
        refresh();
        return;
    }

    setStatus(Status::Listing);

    if (m_pathHistory.isEmpty() || m_pathHistory.last() != resolvedNewUrl)
        m_pathHistory.append(resolvedNewUrl);

    beginResetModel();
    m_url = resolvedNewUrl.toString(QUrl::PreferLocalFile);
    m_dirModel->dirLister()->openUrl(isTrash ? QUrl(QStringLiteral("trash:/")) : resolvedNewUrl);
    clearDragImages();
    m_dragIndexes.clear();
    endResetModel();

    if (isTrash) {
        refresh();
    }

    emit urlChanged();
    emit resolvedUrlChanged();
}

QUrl FolderModel::resolvedUrl() const
{
    return m_dirModel->dirLister()->url();
}

QUrl FolderModel::resolve(const QString &url)
{
    QUrl resolvedUrl;

    if (url.startsWith(QLatin1Char('~'))) {
        resolvedUrl = QUrl::fromLocalFile(QDir::homePath());
    } else {
        resolvedUrl = QUrl::fromUserInput(url);
    }

    return resolvedUrl;
}

FolderModel::Status FolderModel::status() const
{
    return m_status;
}

void FolderModel::setStatus(FolderModel::Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

int FolderModel::sortMode() const
{
    return m_sortMode;
}

void FolderModel::setSortMode(int mode)
{
    if (m_sortMode != mode) {
        m_sortMode = mode;

        if (mode == -1 /* Unsorted */) {
            setDynamicSortFilter(false);
        } else {
            invalidateIfComplete();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
            setDynamicSortFilter(true);
        }

        emit sortModeChanged();
    }
}

bool FolderModel::sortDirsFirst() const
{
    return m_sortDirsFirst;
}

void FolderModel::setSortDirsFirst(bool enable)
{
    if (m_sortDirsFirst != enable) {
        m_sortDirsFirst = enable;

        if (m_sortMode != -1 /* Unsorted */) {
            invalidateIfComplete();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
        }

        emit sortDirsFirstChanged();
    }
}

int FolderModel::filterMode() const
{
    return m_filterMode;
}

void FolderModel::setFilterMode(int filterMode)
{
    if (m_filterMode != (FilterMode)filterMode) {
        m_filterMode = (FilterMode)filterMode;

        invalidateFilterIfComplete();

        emit filterModeChanged();
    }
}

QStringList FolderModel::filterMimeTypes() const
{
    return m_mimeSet.values();
}

void FolderModel::setFilterMimeTypes(const QStringList &mimeList)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QSet<QString> &set = QSet<QString>::fromList(mimeList);
#else
    const QSet<QString> set(mimeList.constBegin(), mimeList.constEnd());
#endif

    if (m_mimeSet != set) {
        m_mimeSet = set;

        invalidateFilterIfComplete();

        emit filterMimeTypesChanged();
    }
}

QString FolderModel::filterPattern() const
{
    return m_filterPattern;
}

void FolderModel::setFilterPattern(const QString &pattern)
{
    if (m_filterPattern == pattern) {
        return;
    }

    m_filterPattern = pattern;
    m_filterPatternMatchAll = (pattern == QLatin1String("*"));

    const QStringList patterns = pattern.split(QLatin1Char(' '));
    m_regExps.clear();
    m_regExps.reserve(patterns.count());

    foreach (const QString &pattern, patterns) {
        QRegExp rx(pattern);
        rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        m_regExps.append(rx);
    }

    invalidateFilterIfComplete();

    emit filterPatternChanged();
}

QObject *FolderModel::viewAdapter() const
{
    return m_viewAdapter;
}

void FolderModel::setViewAdapter(QObject *adapter)
{
    if (m_viewAdapter != adapter) {
        ItemViewAdapter *viewAdapter = dynamic_cast<ItemViewAdapter *>(adapter);
        m_viewAdapter = viewAdapter;
        emit viewAdapterChanged();
    }
}

bool FolderModel::dragging() const
{
    return m_dragInProgress;
}

bool FolderModel::isDir(const QModelIndex &index, const KDirModel *dirModel) const
{
    KFileItem item = dirModel->itemForIndex(index);
    if (item.isDir()) {
        return true;
    }

    return false;
}

bool FolderModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const KDirModel *dirModel = static_cast<KDirModel *>(sourceModel());

    if (m_sortDirsFirst || left.column() == KDirModel::Size) {
        bool leftIsDir = isDir(left, dirModel);
        bool rightIsDir = isDir(right, dirModel);

        if (leftIsDir && !rightIsDir) {
            return (sortOrder() == Qt::AscendingOrder);
        }

        if (!leftIsDir && rightIsDir) {
            return (sortOrder() == Qt::DescendingOrder);
        }
    }

    const KFileItem leftItem = dirModel->data(left, KDirModel::FileItemRole).value<KFileItem>();
    const KFileItem rightItem = dirModel->data(right, KDirModel::FileItemRole).value<KFileItem>();
    const int column = left.column();
    int result = 0;

    switch (column) {
    case KDirModel::Size: {
        if (isDir(left, dirModel) && isDir(right, dirModel)) {
            const int leftChildCount = dirModel->data(left, KDirModel::ChildCountRole).toInt();
            const int rightChildCount = dirModel->data(right, KDirModel::ChildCountRole).toInt();
            if (leftChildCount < rightChildCount)
                result = -1;
            else if (leftChildCount > rightChildCount)
                result = +1;
        } else {
            const KIO::filesize_t leftSize = leftItem.size();
            const KIO::filesize_t rightSize = rightItem.size();
            if (leftSize < rightSize)
                result = -1;
            else if (leftSize > rightSize)
                result = +1;
        }

        break;
    }
    case KDirModel::ModifiedTime: {
        const long long leftTime = leftItem.entry().numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME, -1);
        const long long rightTime = rightItem.entry().numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME, -1);
        if (leftTime < rightTime)
            result = -1;
        else if (leftTime > rightTime)
            result = +1;

        break;
    }
    case KDirModel::Type:
        result = QString::compare(dirModel->data(left, Qt::DisplayRole).toString(), dirModel->data(right, Qt::DisplayRole).toString());
        break;

    default:
        break;
    }

    if (result != 0)
        return result < 0;

    QCollator collator;

    result = collator.compare(leftItem.text(), rightItem.text());

    if (result != 0)
        return result < 0;

    result = collator.compare(leftItem.name(), rightItem.name());

    if (result != 0)
        return result < 0;

    return QString::compare(leftItem.url().url(), rightItem.url().url(), Qt::CaseSensitive);
}

Qt::DropActions FolderModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FolderModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

KFileItem FolderModel::rootItem() const
{
    return m_dirModel->dirLister()->rootItem();
}

int FolderModel::count() const
{
    return rowCount();
}

int FolderModel::selectionCount() const
{
    return m_selectionModel->selectedIndexes().size();
}

QString FolderModel::homePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString FolderModel::desktopPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

QAction *FolderModel::action(const QString &name) const
{
    return m_actionCollection.action(name);
}

void FolderModel::up()
{
    const QUrl &url = KIO::upUrl(resolvedUrl());

    if (url.isValid()) {
        setUrl(url.toString());
    }
}

void FolderModel::goBack()
{
    QUrl url = m_pathHistory.previousPath();

    if (url.isEmpty())
        url = resolvedUrl();

    setUrl(url.toString());
}

void FolderModel::goForward()
{
    QUrl url = m_pathHistory.posteriorPath();

    if (url.isEmpty())
        url = resolvedUrl();

    setUrl(url.toString());
}

void FolderModel::refresh()
{
    m_dirModel->dirLister()->updateDirectory(m_dirModel->dirLister()->url());
}

void FolderModel::undo()
{
    if (KIO::FileUndoManager::self()->undoAvailable()) {
        KIO::FileUndoManager::self()->undo();
    }
}

bool FolderModel::supportSetAsWallpaper(const QString &mimeType)
{
    if (mimeType == "image/jpeg" || mimeType == "image/png")
        return true;

    return false;
}

int FolderModel::fileExtensionBoundary(int row)
{
    const QModelIndex idx = index(row, 0);
    const QString &name = data(idx, Qt::DisplayRole).toString();

    int boundary = name.length();

    if (data(idx, IsDirRole).toBool()) {
        return boundary;
    }

    QMimeDatabase db;
    const QString &ext = db.suffixForFileName(name);

    if (ext.isEmpty()) {
        boundary = name.lastIndexOf(QLatin1Char('.'));

        if (boundary < 1) {
            boundary = name.length();
        }
    } else {
        boundary -= ext.length() + 1;
    }

    return boundary;
}

bool FolderModel::hasSelection() const
{
    return m_selectionModel->hasSelection();
}

bool FolderModel::isSelected(int row) const
{
    if (row < 0)
        return false;

    return m_selectionModel->isSelected(index(row, 0));
}

bool FolderModel::isBlank(int row) const
{
    if (row < 0) {
        return true;
    }

    return data(index(row, 0), BlankRole).toBool();
}

void FolderModel::setSelected(int row)
{
    if (row < 0)
        return;

    m_selectionModel->select(index(row, 0), QItemSelectionModel::Select);
    m_currentIndex = row;

    emit currentIndexChanged();
}

void FolderModel::selectAll()
{
    setRangeSelected(0, rowCount() - 1);
}

void FolderModel::toggleSelected(int row)
{
    if (row < 0)
        return;

    m_selectionModel->select(index(row, 0), QItemSelectionModel::Toggle);
}

void FolderModel::setRangeSelected(int anchor, int to)
{
    if (anchor < 0 || to < 0) {
        return;
    }

    QItemSelection selection(index(anchor, 0), index(to, 0));
    m_selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

void FolderModel::updateSelection(const QVariantList &rows, bool toggle)
{
    QItemSelection newSelection;

    int iRow = -1;

    foreach (const QVariant &row, rows) {
        iRow = row.toInt();

        if (iRow < 0) {
            return;
        }

        const QModelIndex &idx = index(iRow, 0);
        newSelection.select(idx, idx);
    }

    if (toggle) {
        QItemSelection pinnedSelection = m_pinnedSelection;
        pinnedSelection.merge(newSelection, QItemSelectionModel::Toggle);
        m_selectionModel->select(pinnedSelection, QItemSelectionModel::ClearAndSelect);
    } else {
        m_selectionModel->select(newSelection, QItemSelectionModel::ClearAndSelect);
    }
}

void FolderModel::clearSelection()
{
    if (m_selectionModel->hasSelection())
        m_selectionModel->clear();
}

void FolderModel::pinSelection()
{
    m_pinnedSelection = m_selectionModel->selection();
}

void FolderModel::unpinSelection()
{
    m_pinnedSelection = QItemSelection();
}

void FolderModel::newFolder()
{
    QString rootPath = rootItem().url().toString();
    QString baseName = tr("New Folder");
    QString newName = baseName;

    int i = 0;
    while (true) {
        if (QFile::exists(rootItem().url().toLocalFile() + "/" + newName)) {
            ++i;
            newName = QString("%1%2").arg(baseName).arg(QString::number(i));
        } else {
            break;
        }
    }

    m_newDocumentUrl = QUrl(rootItem().url().toString() + "/" + newName);

    auto job = KIO::mkdir(QUrl(rootItem().url().toString() + "/" + newName));
    job->start();
}

void FolderModel::newTextFile()
{
    QString rootPath = rootItem().url().toString();
    QString baseName = tr("New Text");
    QString newName = baseName;

    int i = 0;
    while (true) {
        if (QFile::exists(rootItem().url().toLocalFile() + "/" + newName)) {
            ++i;
            newName = QString("%1%2").arg(baseName).arg(QString::number(i));
        } else {
            break;
        }
    }

    m_newDocumentUrl = QUrl(rootItem().url().toString() + "/" + newName);

    QFile file(m_newDocumentUrl.toLocalFile());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "\n";
        ::chmod(m_newDocumentUrl.toLocalFile().toStdString().c_str(), 0700);
        file.close();
    }
}

void FolderModel::rename(int row, const QString &name)
{
    if (row < 0)
        return;

    QModelIndex idx = index(row, 0);
    m_dirModel->setData(mapToSource(idx), name, Qt::EditRole);
}

void FolderModel::copy()
{
    if (!m_selectionModel->hasSelection())
        return;

    if (QAction *action = m_actionCollection.action("copy"))
        if (!action->isEnabled())
            return;

    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::paste()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    bool enable = false;

    // Update paste action
    if (QAction *paste = m_actionCollection.action(QStringLiteral("paste"))) {
        QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);
        const QString &currentUrl = rootItem().url().toLocalFile();

        if (!urls.isEmpty()) {
            if (!rootItem().isNull()) {
                enable = rootItem().isWritable();
            }
        }

        if (enable && !urls.isEmpty()) {
            for (QUrl url : urls) {
                m_needSelectUrls.append(QUrl::fromLocalFile(QString("%1/%2").arg(currentUrl)
                                                                            .arg(url.fileName())));
            }
        }

        paste->setEnabled(enable);
    }

    if (enable) {
        // Copy a new MimeData.
        QMimeData *data = new QMimeData;
        for (QString mimetype : mimeData->formats()) {
            data->setData(mimetype, mimeData->data(mimetype));
         }

        KIO::Job *job = KIO::paste(data, m_dirModel->dirLister()->url());
        // connect(job, &KIO::Job::finished, this, &FolderModel::delayUpdateNeedSelectUrls);
        job->start();

        // Clear system clipboard.
        if (mimeData->hasFormat("application/x-lingmo-cutselection")) {
            QApplication::clipboard()->clear();
        }
    }
}

void FolderModel::cut()
{
    if (!m_selectionModel->hasSelection())
        return;

    if (QAction *action = m_actionCollection.action("cut"))
        if (!action->isEnabled())
            return;

    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());

    mimeData->setData("application/x-kde-cutselection", QByteArray("1"));
    mimeData->setData("application/x-lingmo-cutselection", QByteArray("1"));

    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::openSelected()
{
    if (!m_selectionModel->hasSelection())
        return;

    if (resolvedUrl().scheme() == QLatin1String("trash"))
        return;

    const QList<QUrl> urls = selectedUrls();
    if (!m_isDesktop) {
        if (urls.size() == 1 && KFileItem(urls.first()).isDir()) {
            setUrl(urls.first().toLocalFile());
            return;
        }
    }

    for (const QUrl &url : urls) {
        KFileItem item(url);
        QString mimeType = item.mimetype();

        // Desktop file.
        if (mimeType == "application/x-desktop") {
            FileLauncher::self()->launchApp(url.toLocalFile(), "");
            continue;
        }

        // runnable
        if (mimeType == "application/x-executable" ||
            mimeType == "application/x-sharedlib" ||
            mimeType == "application/x-iso9660-appimage" ||
            mimeType == "application/vnd.appimage") {
            QFileInfo fileInfo(url.toLocalFile());
            if (!fileInfo.isExecutable()) {
                QFile file(url.toLocalFile());
                file.setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
            }

            FileLauncher::self()->launchExecutable(url.toLocalFile());

            continue;
        }

        QString defaultAppDesktopFile = m_mimeAppManager->getDefaultAppByMimeType(item.currentMimeType());

        // If no default application is found,
        // look for the first one of the frequently used applications.
        if (defaultAppDesktopFile.isEmpty()) {
            QStringList recommendApps = m_mimeAppManager->getRecommendedAppsByMimeType(item.currentMimeType());
            if (recommendApps.count() > 0) {
                defaultAppDesktopFile = recommendApps.first();
            }
        }

        if (!defaultAppDesktopFile.isEmpty()) {
            FileLauncher::self()->launchApp(defaultAppDesktopFile, url.toLocalFile());
            continue;
        }

        QDesktopServices::openUrl(url);
    }
}

void FolderModel::showOpenWithDialog()
{
    if (!m_selectionModel->hasSelection())
        return;

    const QList<QUrl> urls = selectedUrls();

    OpenWithDialog *dlg = new OpenWithDialog(urls.first());
    dlg->show();
}

void FolderModel::deleteSelected()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (QAction *action = m_actionCollection.action(QStringLiteral("del"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    KIO::DeleteJob *job = KIO::del(selectedUrls());
    job->start();
}

void FolderModel::moveSelectedToTrash()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (QAction *action = m_actionCollection.action(QStringLiteral("trash"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    const QList<QUrl> urls = selectedUrls();
    KIO::JobUiDelegate uiDelegate;

    if (uiDelegate.askDeleteConfirmation(urls, KIO::JobUiDelegate::Trash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job *job = KIO::trash(urls);
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
        KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, urls, QUrl(QStringLiteral("trash:/")), job);
    }
}

void FolderModel::emptyTrash()
{
    QProcess::startDetached("lingmo-filemanager", QStringList() << "-e");
}

void FolderModel::keyDeletePress()
{
    if (!m_selectionModel->hasSelection())
        return;

    resolvedUrl().scheme() == "trash" ? openDeleteDialog() : moveSelectedToTrash();
}

void FolderModel::setDragHotSpotScrollOffset(int x, int y)
{
    m_dragHotSpotScrollOffset.setX(x);
    m_dragHotSpotScrollOffset.setY(y);
}

void FolderModel::addItemDragImage(int row, int x, int y, int width, int height, const QVariant &image)
{
    if (row < 0)
        return;

    delete m_dragImages.take(row);

    DragImage *dragImage = new DragImage();
    dragImage->row = row;
    dragImage->rect = QRect(x, y, width, height);
    dragImage->image = image.value<QImage>();
    dragImage->blank = false;

    m_dragImages.insert(row, dragImage);
}

void FolderModel::clearDragImages()
{
    qDeleteAll(m_dragImages);
    m_dragImages.clear();
}

void FolderModel::dragSelected(int x, int y)
{
    if (m_dragInProgress)
        return;

    m_dragInProgress = true;
    emit draggingChanged();

    QMetaObject::invokeMethod(this, "dragSelectedInternal", Qt::QueuedConnection, Q_ARG(int, x), Q_ARG(int, y));
}

void FolderModel::drop(QQuickItem *target, QObject *dropEvent, int row)
{
    QMimeData *mimeData = qobject_cast<QMimeData *>(dropEvent->property("mimeData").value<QObject *>());

    if (!mimeData) {
        return;
    }

    QModelIndex idx;
    KFileItem item;

    if (row > -1 && row < rowCount()) {
        idx = index(row, 0);
        item = itemForIndex(idx);
    }

    QUrl dropTargetUrl;

    // So we get to run mostLocalUrl() over the current URL.
    if (item.isNull()) {
        item = rootItem();
    }

    if (item.isNull()) {
        dropTargetUrl = m_dirModel->dirLister()->url();
    } else {
        dropTargetUrl = item.mostLocalUrl();
    }

    auto dropTargetFolderUrl = dropTargetUrl;
    if (dropTargetFolderUrl.fileName() == QLatin1Char('.')) {
        // the target URL for desktop:/ is e.g. 'file://home/user/Desktop/.'
        dropTargetFolderUrl = dropTargetFolderUrl.adjusted(QUrl::RemoveFilename);
    }

    const int x = dropEvent->property("x").toInt();
    const int y = dropEvent->property("y").toInt();
    const QPoint dropPos = {x, y};

    if (m_dragInProgress && row == -1) {
        if (mimeData->urls().isEmpty())
            return;

        setSortMode(-1);

        for (const auto &url : mimeData->urls()) {
            m_dropTargetPositions.insert(url.fileName(), dropPos);
        }

        emit move(x, y, mimeData->urls());

        return;
    }


    if (idx.isValid() && !(flags(idx) & Qt::ItemIsDropEnabled)) {
        return;
    }

    if (!isDropBetweenSharedViews(mimeData->urls(), dropTargetFolderUrl)) {
        KIO::Job *job = KIO::move(mimeData->urls(), dropTargetUrl, KIO::HideProgressInfo);
        job->start();

        // Add select
        for (QUrl url : mimeData->urls()) {
            m_needSelectUrls.append(QUrl::fromLocalFile(QString("%1/%2").arg(rootItem().url().toLocalFile())
                                                                        .arg(url.fileName())));
        }
    }
}

void FolderModel::setWallpaperSelected()
{
    if (!m_selectionModel)
        return;

    QUrl url = selectedUrls().first();

    if (!url.isLocalFile())
        return;

    QDBusInterface iface("com.lingmo.Settings", "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus(), nullptr);
    if (iface.isValid())
        iface.call("setWallpaper", url.toLocalFile());
}

void FolderModel::setLoginWallpaperSelected()
{
    if (!m_selectionModel)
        return;

    QUrl url = selectedUrls().first();

    if (!url.isLocalFile())
        return;

    // Define the target directory and new file name
    QString srcFilePath = selectedUrls().first().toLocalFile();
    qDebug() << srcFilePath;
    QString targetDir = "/etc/system/wallpaper/";
    QString targetFilePath = targetDir + "img.jpg";
    QProcess::startDetached("pkexec", QStringList() << "cp" << srcFilePath << targetFilePath);
}

void FolderModel::openContextMenu(QQuickItem *visualParent, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    updateActions();

    const QModelIndexList indexes = m_selectionModel->selectedIndexes();
    const bool isTrash = (resolvedUrl().scheme() == QLatin1String("trash"));
    QMenu *menu = new QMenu;

    // Open folder menu.
    if (indexes.isEmpty()) {
        QAction *selectAll = new QAction(tr("Select All"), this);
        selectAll->setIcon(QIcon::fromTheme("edit-select-all")); // 添加全选图标
        connect(selectAll, &QAction::triggered, this, &FolderModel::selectAll);

        menu->addAction(m_actionCollection.action("newFolder"));

        if (!isTrash) {
            QMenu *newMenu = new QMenu(tr("New Documents"));
            newMenu->setIcon(QIcon::fromTheme("document-new")); // 添加新建文档图标
            newMenu->menuAction()->setIconVisibleInMenu(true);
            newMenu->addAction(m_actionCollection.action("newTextFile"));
            menu->addMenu(newMenu);
        }

        // 添加刷新菜单
        QAction *refresh = new QAction(tr("Refresh"), this);
        refresh->setIcon(QIcon::fromTheme("view-refresh"));
        connect(refresh, &QAction::triggered, this, [this]() {
            m_dirLister->updateDirectory(m_dirLister->url());
        });

        menu->addSeparator();
        menu->addAction(refresh);  // 添加到空白区域的右键菜单
        menu->addAction(m_actionCollection.action("paste"));
        menu->addAction(selectAll);
        if (m_actionCollection.action("terminal")->isVisible()) {
            menu->addSeparator();
            menu->addAction(m_actionCollection.action("terminal"));
        }

        if (m_isDesktop) {
            menu->addAction(m_actionCollection.action("changeBackground"));
            menu->addAction(m_actionCollection.action("iconSizeMenu"));
        }

        menu->addSeparator();
        menu->addAction(m_actionCollection.action("showHidden"));

        menu->addSeparator();
        menu->addAction(m_actionCollection.action("emptyTrash"));
        menu->addAction(m_actionCollection.action("properties"));

        menu->addSeparator();
        menu->addAction(m_actionCollection.action("showHidden"));

        menu->addSeparator();
        menu->addAction(m_actionCollection.action("emptyTrash"));
        menu->addAction(m_actionCollection.action("properties"));
    } else {
        // Open the items menu.

        // Trash items
        menu->addAction(m_actionCollection.action("restore"));

        menu->addAction(m_actionCollection.action("open"));
        menu->addAction(m_actionCollection.action("openInNewWindow"));

        menu->addAction(m_actionCollection.action("openWith"));
        menu->addSeparator();
        menu->addAction(m_actionCollection.action("cut"));
        menu->addAction(m_actionCollection.action("copy"));
        menu->addAction(m_actionCollection.action("trash"));
        menu->addAction(m_actionCollection.action("del"));
        menu->addAction(m_actionCollection.action("rename"));

        if (m_actionCollection.action("terminal")->isVisible()) {
            menu->addSeparator();
            menu->addAction(m_actionCollection.action("terminal"));
        }

        menu->addAction(m_actionCollection.action("setWallpaperMenu"));
        menu->addSeparator();
        menu->addAction(m_actionCollection.action("properties"));
    }

    // 添加压缩/解压缩菜单
    if (!indexes.isEmpty()) {
        const KFileItem item = itemForIndex(indexes.first());
        const QString mimetype = item.mimetype();
        
        // 支持的压缩文件 MIME 类型列表
        QStringList supportedMimeTypes = {
            "application/x-lrzip",
            "application/x-cd-image",
            "application/x-iso9660-appimage", 
            "application/x-lzip-compressed-tar",
            "application/x-bcpio",
            "application/x-lzop",
            "application/x-archive",
            "application/x-source-rpm",
            "application/x-sv4cpio",
            "application/x-compress",
            "application/x-xar",
            "application/x-xz",
            "application/x-tar",
            "application/x-tarz",
            "application/zstd",
            "application/x-lzma",
            "application/x-lzip",
            "application/x-xz-compressed-tar",
            "application/gzip",
            "application/x-lz4",
            "application/x-rpm",
            "application/x-bzip-compressed-tar",
            "application/x-bzip",
            "application/x-deb",
            "application/zlib",
            "application/vnd.ms-cab-compressed",
            "application/x-lzma-compressed-tar",
            "application/x-cpio",
            "application/x-java-archive",
            "application/zip",
            "application/x-cpio-compressed",
            "application/x-zstd-compressed-tar",
            "application/vnd.debian.binary-package",
            "application/x-compressed-tar",
            "application/x-sv4crc"
        };
        
        if (supportedMimeTypes.contains(mimetype)) {
            // 如果是支持的压缩文件，添加解压菜单
            menu->addSeparator();
            QMenu *extractMenu = menu->addMenu(tr("Extract"));
            extractMenu->setIcon(QIcon::fromTheme("archive-extract"));

            // 添加"解压到当前文件夹"选项
            QAction *extractHere = new QAction(tr("Extract Here"), this);
            extractHere->setIcon(QIcon::fromTheme("archive-extract"));
            connect(extractHere, &QAction::triggered, this, [this]() {
                QString archivePath = selectedUrls().first().toLocalFile();
                QString destPath = QFileInfo(archivePath).path();
                QProcess::startDetached("ark", QStringList() << "--batch" << "--autosubfolder" 
                                     << "--destination" << destPath << archivePath);
            });
            extractMenu->addAction(extractHere);

            // 添加"解压到..."选项
            QAction *extractTo = new QAction(tr("Extract To..."), this);
            extractTo->setIcon(QIcon::fromTheme("archive-extract"));
            connect(extractTo, &QAction::triggered, this, [this]() {
                QString archivePath = selectedUrls().first().toLocalFile();
                QString destPath = QFileDialog::getExistingDirectory(nullptr, tr("Select Destination"));
                if (!destPath.isEmpty()) {
                    QProcess::startDetached("ark", QStringList() << "--batch" << "--autosubfolder" 
                                         << "--destination" << destPath << archivePath);
                }
            });
            extractMenu->addAction(extractTo);
        } else {
            // 如果不是压缩文件，添加压缩菜单
            menu->addSeparator();
            QMenu *compressMenu = menu->addMenu(tr("Compress"));
            compressMenu->setIcon(QIcon::fromTheme("archive-insert"));

            // 添加常用压缩格式
            struct CompressFormat {
                QString name;
                QString extension;
                QString mimeType;
            };
            
            QAction *zipAction = new QAction(tr("ZIP Archive"), this);
            zipAction->setIcon(QIcon::fromTheme("archive-insert"));
            connect(zipAction, &QAction::triggered, this, [this]() {
                QStringList files;
                for (const QUrl &url : selectedUrls()) {
                    files << url.toLocalFile();
                }
                QString baseName = QFileInfo(files.first()).baseName();
                QString archivePath = QFileInfo(files.first()).path() + "/" + baseName + ".zip";
                QProcess::startDetached("ark", QStringList() << "--batch" << "--add-to" << archivePath << files);
            });
            compressMenu->addAction(zipAction);

            QAction *sevenZAction = new QAction(tr("7-Zip Archive"), this);
            sevenZAction->setIcon(QIcon::fromTheme("archive-insert"));
            connect(sevenZAction, &QAction::triggered, this, [this]() {
                QStringList files;
                for (const QUrl &url : selectedUrls()) {
                    files << url.toLocalFile();
                }
                QString baseName = QFileInfo(files.first()).baseName();
                QString archivePath = QFileInfo(files.first()).path() + "/" + baseName + ".7z";
                QProcess::startDetached("ark", QStringList() << "--batch" << "--add-to" << archivePath << files);
            });
            compressMenu->addAction(sevenZAction);

            // TAR 子菜单
            QMenu *tarMenu = new QMenu(tr("TAR Archive"), compressMenu);
            tarMenu->setIcon(QIcon::fromTheme("archive-insert"));

            // TAR 压缩选项
            struct TarFormat {
                QString name;
                QString extension;
                QString mimeType;
            };

            QList<TarFormat> tarFormats = {
                {tr("TAR Archive (gzip)"), "tar.gz", "application/x-compressed-tar"},
                {tr("TAR Archive (bzip2)"), "tar.bz2", "application/x-bzip-compressed-tar"},
                {tr("TAR Archive (xz)"), "tar.xz", "application/x-xz-compressed-tar"},
                {tr("TAR Archive (zstd)"), "tar.zst", "application/x-zstd-compressed-tar"},
                {tr("TAR Archive (uncompressed)"), "tar", "application/x-tar"}
            };

            for (const auto &format : tarFormats) {
                QAction *action = new QAction(format.name, this);
                action->setIcon(QIcon::fromTheme("archive-insert"));
                connect(action, &QAction::triggered, this, [this, format]() {
                    QStringList files;
                    for (const QUrl &url : selectedUrls()) {
                        files << url.toLocalFile();
                    }
                    
                    QString baseName = QFileInfo(files.first()).baseName();
                    QString archivePath = QFileInfo(files.first()).path() + "/" + baseName + "." + format.extension;
                    
                    QProcess::startDetached("ark", QStringList() << "--batch" << "--add-to" << archivePath << files);
                });
                compressMenu->addAction(action);
            }
        }
    }

    QPoint position;
    if (visualParent) {
        position = visualParent->mapToGlobal(QPointF(0, visualParent->height())).toPoint();
    } else {
        position = QCursor::pos();
    }

    menu->installEventFilter(this);
    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->winId();
    menu->popup(position);
    connect(menu, &QMenu::aboutToHide, [menu]() {
        menu->deleteLater();
    });
}

void FolderModel::openPropertiesDialog()
{
    const QModelIndexList indexes = m_selectionModel->selectedIndexes();

    if (indexes.isEmpty()) {
        FilePropertiesDialog *dlg = new FilePropertiesDialog(QUrl::fromLocalFile(url()));
        dlg->show();
        return;
    }

    KFileItemList items;
    items.reserve(indexes.count());
    for (const QModelIndex &index : indexes) {
        KFileItem item = itemForIndex(index);
        if (!item.isNull()) {
            items.append(item);
        }
    }

    FilePropertiesDialog *dlg = new FilePropertiesDialog(items);
    dlg->show();
}

void FolderModel::openInTerminal()
{
    QString url;
    if (m_selectionModel->hasSelection()) {
        KFileItem item = itemForIndex(m_selectionModel->selectedIndexes().first());
        if (item.isDir()) {
            url = item.url().toLocalFile();
        }
    } else {
        url = rootItem().url().toLocalFile();
    }

    m_mimeAppManager->launchTerminal(url);
}

void FolderModel::openChangeWallpaperDialog()
{
    QProcess::startDetached("lingmo-settings", QStringList() << "-m" << "background");
}

void FolderModel::openDeleteDialog()
{
    Window *w = new Window;
    w->load(QUrl("qrc:/qml/Dialogs/DeleteDialog.qml"));
    w->rootContext()->setContextProperty("model", this);
}

void FolderModel::openInNewWindow(const QString &url)
{
    if (!url.isEmpty()) {
        QProcess::startDetached("lingmo-filemanager", QStringList() << url);
        return;
    }

    // url 为空则打开已选择的 items.
    if (!m_selectionModel->hasSelection())
        return;

    for (const QModelIndex &index : m_selectionModel->selectedIndexes()) {
        KFileItem item = itemForIndex(index);
        if (item.isDir()) {
            QProcess::startDetached("lingmo-filemanager", QStringList() << item.url().toLocalFile());
        }
    }
}

void FolderModel::updateSelectedItemsSize()
{
}

void FolderModel::keyboardSearch(const QString &text)
{
    if (rowCount() == 0)
        return;

    int index;
    int currentIndex = -1;

    if (m_selectionModel->hasSelection()) {
        currentIndex = m_selectionModel->selectedIndexes().first().row();
    }

    index = indexForKeyboardSearch(text, (currentIndex + 1) % rowCount());

    if (index < 0 || currentIndex == index)
        return;

    if (index >= 0) {
        clearSelection();
        setSelected(index);

        emit scrollToItem(index);
    }
}

void FolderModel::clearPixmapCache()
{
    QPixmapCache::clear();
}

void FolderModel::restoreFromTrash()
{
    if (!m_selectionModel->hasSelection())
        return;

    if (QAction *action = m_actionCollection.action("restore"))
        if (!action->isVisible())
            return;


    KIO::RestoreJob *job = KIO::restoreFromTrash(selectedUrls());
    job->start();
}

void FolderModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList indices = selected.indexes();
    indices.append(deselected.indexes());

    QVector<int> roles;
    roles.append(SelectedRole);

    foreach (const QModelIndex &index, indices) {
        emit dataChanged(index, index, roles);
    }

    if (!m_selectionModel->hasSelection()) {
        clearDragImages();
    } else {
        foreach (const QModelIndex &idx, deselected.indexes()) {
            delete m_dragImages.take(idx.row());
        }
    }

    updateActions();

    emit selectionCountChanged();

    // The desktop does not need to calculate the selected file size.
    if (m_isDesktop)
        return;

    // Start calculating file size.
    if (m_sizeJob == nullptr) {
        m_sizeJob = new CFileSizeJob;

        connect(m_sizeJob, &CFileSizeJob::sizeChanged, this, [=] {
            m_selectedItemSize = KIO::convertSize(m_sizeJob->totalSize());
            if (!m_selectionModel->hasSelection())
                m_selectedItemSize = "";
            emit selectedItemSizeChanged();
        });

        connect(m_sizeJob, &CFileSizeJob::result, this, [=] {
            m_selectedItemSize = KIO::convertSize(m_sizeJob->totalSize());
            if (!m_selectionModel->hasSelection())
                m_selectedItemSize = "";
            emit selectedItemSizeChanged();
        });
    }

    m_sizeJob->stop();

    if (!m_selectionModel->hasSelection()) {
        m_sizeJob->blockSignals(true);
        m_selectedItemSize = "";
        emit selectedItemSizeChanged();
    } else {
        bool fileExists = false;

        for (const QModelIndex &index : m_selectionModel->selectedIndexes()) {
            if (itemForIndex(index).isFile()) {
                fileExists = true;
                break;
            }
        }

        // Reion: The size label at the bottom needs to be updated
        // only if you select the include file.
        if (fileExists) {
            m_sizeJob->blockSignals(false);
            m_sizeJob->start(selectedUrls());
        }
    }
}

void FolderModel::dragSelectedInternal(int x, int y)
{
    if (!m_viewAdapter || !m_selectionModel->hasSelection()) {
        m_dragInProgress = false;
        emit draggingChanged();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem *>(m_viewAdapter->adapterView());
    QDrag *drag = new QDrag(item);
    addDragImage(drag, x, y);

    m_dragIndexes = m_selectionModel->selectedIndexes();
    std::sort(m_dragIndexes.begin(), m_dragIndexes.end());

    // TODO: Optimize to emit contiguous groups.
    emit dataChanged(m_dragIndexes.first(), m_dragIndexes.last(), QVector<int>() << BlankRole);

    QModelIndexList sourceDragIndexes;
    sourceDragIndexes.reserve(m_dragIndexes.count());
    foreach (const QModelIndex &index, m_dragIndexes) {
        sourceDragIndexes.append(mapToSource(index));
    }

    drag->setMimeData(m_dirModel->mimeData(sourceDragIndexes));

    // Due to spring-loading (aka auto-expand), the URL might change
    // while the drag is in-flight - in that case we don't want to
    // unnecessarily emit dataChanged() for (possibly invalid) indices
    // after it ends.
    const QUrl currentUrl(m_dirModel->dirLister()->url());

    item->grabMouse();
    drag->exec(supportedDragActions());
    item->ungrabMouse();

    m_dragInProgress = false;
    emit draggingChanged();

    if (m_dirModel->dirLister()->url() == currentUrl) {
        const QModelIndex first(m_dragIndexes.first());
        const QModelIndex last(m_dragIndexes.last());
        m_dragIndexes.clear();
        // TODO: Optimize to emit contiguous groups.
        emit dataChanged(first, last, QVector<int>() << BlankRole);
    }
}

void FolderModel::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (m_updateNeedSelectTimer->isActive()) {
        m_updateNeedSelectTimer->stop();
    }

    QModelIndex changeIdx;

    for (int i = first; i <= last; ++i) {
        const auto idx = index(i, 0, parent);
        const auto url = itemForIndex(idx).url();
        auto it = m_dropTargetPositions.find(url.fileName());
        if (it != m_dropTargetPositions.end()) {
            const auto pos = it.value();
            m_dropTargetPositions.erase(it);
            Q_EMIT move(pos.x(), pos.y(), {url});
        }

        if (url == m_newDocumentUrl) {
            changeIdx = idx;
            m_newDocumentUrl.clear();
        }
    }

    // 新建文件夹需要先选择后再发送请求
    QTimer::singleShot(m_updateNeedSelectTimer->interval() + 10, this, [=] {
        if (changeIdx.isValid()) {
            setSelected(changeIdx.row());
            emit requestRename();
        }
    });

    m_updateNeedSelectTimer->start();
}

void FolderModel::delayUpdateNeedSelectUrls()
{
    QTimer::singleShot(100, this, &FolderModel::updateNeedSelectUrls);
}

void FolderModel::updateNeedSelectUrls()
{
    QModelIndexList needSelectList;

    for (const QUrl &url : m_needSelectUrls) {
        const QModelIndex &idx = indexForUrl(url);

        if (!idx.isValid())
            continue;

        const QModelIndex &sourceIdx = mapFromSource(idx);

        needSelectList.append(sourceIdx);
    }

    m_needSelectUrls.clear();

    // If the selected item already exists, clear it immediately.
    if (!needSelectList.isEmpty()) {
        clearSelection();
    }

    for (const QModelIndex &idx : needSelectList) {
        setSelected(idx.row());
    }
}

bool FolderModel::isSupportThumbnails(const QString &mimeType) const
{
    const QStringList supportsMimetypes = {"image/bmp", "image/png", "image/gif", "image/jpeg", "image/web",
                                           /*"application/pdf", "application/rtf", "application/doc", "application/odf",
                                           "audio/mpeg", "video/mp4"*/};

    // 直接检查传入的 mimeType 参数
    return supportsMimetypes.contains(mimeType);
}

bool FolderModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const KDirModel *dirModel = static_cast<KDirModel *>(sourceModel());
    const KFileItem item = dirModel->itemForIndex(dirModel->index(sourceRow, KDirModel::Name, sourceParent));

    if (m_filterMode == NoFilter) {
        return true;
    }

    if (m_filterMode == FilterShowMatches) {
        return (matchPattern(item) && matchMimeType(item));
    } else {
        return !(matchPattern(item) && matchMimeType(item));
    }
}

bool FolderModel::matchMimeType(const KFileItem &item) const
{
    if (m_mimeSet.isEmpty()) {
        return false;
    }

    if (m_mimeSet.contains(QLatin1String("all/all")) || m_mimeSet.contains(QLatin1String("all/allfiles"))) {
        return true;
    }

    const QString mimeType = item.determineMimeType().name();
    return m_mimeSet.contains(mimeType);
}

bool FolderModel::matchPattern(const KFileItem &item) const
{
    if (m_filterPatternMatchAll) {
        return true;
    }

    const QString name = item.name();
    QListIterator<QRegExp> i(m_regExps);
    while (i.hasNext()) {
        if (i.next().exactMatch(name)) {
            return true;
        }
    }

    return false;
}

bool FolderModel::showHiddenFiles() const
{
    return m_showHiddenFiles;
}

void FolderModel::setShowHiddenFiles(bool showHiddenFiles)
{
    if (m_showHiddenFiles != showHiddenFiles) {
        m_showHiddenFiles = showHiddenFiles;

        m_dirLister->setShowingDotFiles(m_showHiddenFiles);
        m_dirLister->emitChanges();

        QSettings settings("lingmoos", qApp->applicationName());
        settings.setValue("showHiddenFiles", m_showHiddenFiles);

        emit showHiddenFilesChanged();
    }
}

QString FolderModel::selectedItemSize() const
{
    return m_selectedItemSize;
}

bool FolderModel::isDesktop() const
{
    return m_isDesktop;
}

void FolderModel::setIsDesktop(bool isDesktop)
{
    if (m_isDesktop == isDesktop)
        return;

    m_isDesktop = isDesktop;
    emit isDesktopChanged();
}

void FolderModel::invalidateIfComplete()
{
    if (!m_complete)
        return;

    invalidate();
}

void FolderModel::invalidateFilterIfComplete()
{
    if (!m_complete)
        return;

    invalidateFilter();
}

void FolderModel::createActions()
{
    QAction *open = new QAction(tr("Open"), this);
    open->setIcon(QIcon::fromTheme("document-open"));
    connect(open, &QAction::triggered, this, &FolderModel::openSelected);

    QAction *openWith = new QAction(tr("Open with"), this);
    openWith->setIcon(QIcon::fromTheme("document-open-with")); 
    connect(openWith, &QAction::triggered, this, &FolderModel::showOpenWithDialog);

    QAction *cut = new QAction(tr("Cut"), this);
    cut->setIcon(QIcon::fromTheme("edit-cut"));
    connect(cut, &QAction::triggered, this, &FolderModel::cut);

    QAction *copy = new QAction(tr("Copy"), this);
    copy->setIcon(QIcon::fromTheme("edit-copy"));
    connect(copy, &QAction::triggered, this, &FolderModel::copy);

    QAction *paste = new QAction(tr("Paste"), this);
    paste->setIcon(QIcon::fromTheme("edit-paste"));
    connect(paste, &QAction::triggered, this, &FolderModel::paste);

    QAction *newFolder = new QAction(tr("New Folder"), this);
    newFolder->setIcon(QIcon::fromTheme("folder-new"));
    connect(newFolder, &QAction::triggered, this, &FolderModel::newFolder);

    QMenu *newDocuments = new QMenu(tr("New Documents"));
    newDocuments->setIcon(QIcon::fromTheme("document-new"));
    newDocuments->menuAction()->setIconVisibleInMenu(true);
    QAction *newTextFile = new QAction(tr("New Text"), this);
    newTextFile->setIcon(QIcon::fromTheme("text-x-generic"));
    newTextFile->setIconVisibleInMenu(true);
    connect(newTextFile, &QAction::triggered, this, &FolderModel::newTextFile);
    newDocuments->addAction(newTextFile);

    QAction *trash = new QAction(tr("Move To Trash"), this);
    trash->setIcon(QIcon::fromTheme("user-trash"));
    connect(trash, &QAction::triggered, this, &FolderModel::moveSelectedToTrash);

    QAction *emptyTrash = new QAction(tr("Empty Trash"), this);
    emptyTrash->setIcon(QIcon::fromTheme("trash-empty"));
    connect(emptyTrash, &QAction::triggered, this, &FolderModel::emptyTrash);

    QAction *del = new QAction(tr("Delete"), this);
    del->setIcon(QIcon::fromTheme("edit-delete"));
    connect(del, &QAction::triggered, this, &FolderModel::openDeleteDialog);

    QAction *rename = new QAction(tr("Rename"), this);
    rename->setIcon(QIcon::fromTheme("edit-rename"));
    connect(rename, &QAction::triggered, this, &FolderModel::requestRename);

    QAction *terminal = new QAction(tr("Open in Terminal"), this);
    terminal->setIcon(QIcon::fromTheme("utilities-terminal"));
    connect(terminal, &QAction::triggered, this, &FolderModel::openInTerminal);

    QMenu *setWallpaperMenu = new QMenu(tr("Set as ..."));
    setWallpaperMenu->setIcon(QIcon::fromTheme("preferences-desktop-wallpaper"));
    setWallpaperMenu->menuAction()->setIconVisibleInMenu(true);

    QAction *setWallpaper = new QAction(tr("Set as Wallpaper"), this);
    setWallpaper->setIcon(QIcon::fromTheme("preferences-desktop-wallpaper"));
    setWallpaper->setIconVisibleInMenu(true);
    connect(setWallpaper, &QAction::triggered, this, &FolderModel::setWallpaperSelected);
    setWallpaperMenu->addAction(setWallpaper);

    QAction *wallpaperLogin = new QAction(tr("Set as Login Screen"), this);
    wallpaperLogin->setIcon(QIcon::fromTheme("preferences-desktop-wallpaper"));
    wallpaperLogin->setIconVisibleInMenu(true);
    connect(wallpaperLogin, &QAction::triggered, this, &FolderModel::setLoginWallpaperSelected);
    setWallpaperMenu->addAction(wallpaperLogin);

    QAction *properties = new QAction(tr("Properties"), this);
    properties->setIcon(QIcon::fromTheme("document-properties"));
    QObject::connect(properties, &QAction::triggered, this, &FolderModel::openPropertiesDialog);

    QAction *changeBackground = new QAction(tr("Change background"), this);
    changeBackground->setIcon(QIcon::fromTheme("preferences-desktop-wallpaper"));
    QObject::connect(changeBackground, &QAction::triggered, this, &FolderModel::openChangeWallpaperDialog);

    QAction *restore = new QAction(tr("Restore"), this);
    restore->setIcon(QIcon::fromTheme("edit-undo"));
    QObject::connect(restore, &QAction::triggered, this, &FolderModel::restoreFromTrash);

    QAction *showHidden = new QAction(tr("Show hidden files"), this);
    showHidden->setIcon(QIcon::fromTheme("view-hidden"));
    QObject::connect(showHidden, &QAction::triggered, this, [=] {
        setShowHiddenFiles(!m_showHiddenFiles);
    });

    QAction *openInNewWindow = new QAction(tr("Open in new window"), this);
    openInNewWindow->setIcon(QIcon::fromTheme("window-new"));
    QObject::connect(openInNewWindow, &QAction::triggered, this, [=] { this->openInNewWindow(); });

    QMenu *iconSizeMenu = new QMenu(tr("Icon Size"));
    iconSizeMenu->setIcon(QIcon::fromTheme("preferences-desktop-icons"));
    iconSizeMenu->menuAction()->setIconVisibleInMenu(true);
    
    QAction *hugeIcon = new QAction(tr("Huge"), this);
    hugeIcon->setIcon(QIcon::fromTheme("zoom-in"));
    hugeIcon->setIconVisibleInMenu(true);
    
    QAction *largeIcon = new QAction(tr("Large"), this);
    largeIcon->setIcon(QIcon::fromTheme("zoom-in"));
    largeIcon->setIconVisibleInMenu(true);
    
    QAction *normalIcon = new QAction(tr("Normal"), this);
    normalIcon->setIcon(QIcon::fromTheme("zoom-original"));
    normalIcon->setIconVisibleInMenu(true);
    
    QAction *smallIcon = new QAction(tr("Small"), this);
    smallIcon->setIcon(QIcon::fromTheme("zoom-out"));
    smallIcon->setIconVisibleInMenu(true);

    connect(hugeIcon, &QAction::triggered, this, [=] {
        emit changeIconSize(96); // hugeIconSize
    });
    connect(largeIcon, &QAction::triggered, this, [=] {
        emit changeIconSize(72); // largeIconSize  
    });
    connect(normalIcon, &QAction::triggered, this, [=] {
        emit changeIconSize(64); // normalIconSize
    });
    connect(smallIcon, &QAction::triggered, this, [=] {
        emit changeIconSize(48); // smallIconSize
    });

    iconSizeMenu->addAction(hugeIcon);
    iconSizeMenu->addAction(largeIcon); 
    iconSizeMenu->addAction(normalIcon);
    iconSizeMenu->addAction(smallIcon);

    QMenu *viewMenu = new QMenu(tr("View"));
    viewMenu->addMenu(iconSizeMenu);
    viewMenu->addSeparator();
    viewMenu->addAction(showHidden);

    m_actionCollection.addAction(QStringLiteral("iconSizeMenu"), iconSizeMenu->menuAction());

    m_actionCollection.addAction(QStringLiteral("open"), open);
    m_actionCollection.addAction(QStringLiteral("openWith"), openWith);
    m_actionCollection.addAction(QStringLiteral("cut"), cut);
    m_actionCollection.addAction(QStringLiteral("copy"), copy);
    m_actionCollection.addAction(QStringLiteral("paste"), paste);
    m_actionCollection.addAction(QStringLiteral("newFolder"), newFolder);
    m_actionCollection.addAction(QStringLiteral("newTextFile"), newTextFile);
    m_actionCollection.addAction(QStringLiteral("trash"), trash);
    m_actionCollection.addAction(QStringLiteral("emptyTrash"), emptyTrash);
    m_actionCollection.addAction(QStringLiteral("del"), del);
    m_actionCollection.addAction(QStringLiteral("rename"), rename);
    m_actionCollection.addAction(QStringLiteral("terminal"), terminal);
    m_actionCollection.addAction(QStringLiteral("setWallpaperMenu"), setWallpaperMenu->menuAction());
    m_actionCollection.addAction(QStringLiteral("setWallpaper"), setWallpaper);
    m_actionCollection.addAction(QStringLiteral("wallpaperLogin"), wallpaperLogin);
    m_actionCollection.addAction(QStringLiteral("properties"), properties);
    m_actionCollection.addAction(QStringLiteral("changeBackground"), changeBackground);
    m_actionCollection.addAction(QStringLiteral("restore"), restore);
    m_actionCollection.addAction(QStringLiteral("showHidden"), showHidden);
    m_actionCollection.addAction(QStringLiteral("openInNewWindow"), openInNewWindow);
    m_actionCollection.addAction(QStringLiteral("viewMenu"), viewMenu->menuAction());

    // 添加刷新菜单
    QAction *refresh = new QAction(tr("Refresh"), this);
    refresh->setIcon(QIcon::fromTheme("view-refresh"));
    m_actionCollection.addAction(QStringLiteral("refresh"), refresh);
}

void FolderModel::updateActions()
{
    const QModelIndexList indexes = m_selectionModel->selectedIndexes();

    KFileItemList items;
    QList<QUrl> urls;
    bool hasRemoteFiles = false;
    bool isTrashLink = false;
    bool hasDir = false;
    const bool isTrash = (resolvedUrl().scheme() == QLatin1String("trash"));

    if (indexes.isEmpty()) {
        items << rootItem();
    } else {
        items.reserve(indexes.count());
        urls.reserve(indexes.count());
        for (const QModelIndex &index : indexes) {
            KFileItem item = itemForIndex(index);
            if (!item.isNull()) {
                hasRemoteFiles |= item.localPath().isEmpty();
                items.append(item);
                urls.append(item.url());
            }

            if (item.isDir())
                hasDir = true;
        }
    }

    KFileItemListProperties itemProperties(items);
    // Check if we're showing the menu for the trash link
    if (items.count() == 1 && items.at(0).isDesktopFile()) {
        KDesktopFile file(items.at(0).localPath());
        if (file.hasLinkType() && file.readUrl() == QLatin1String("trash:/")) {
            isTrashLink = true;
        }
    }

    if (QAction *openAction = m_actionCollection.action(QStringLiteral("open"))) {
        openAction->setVisible(!isTrash);
    }

    if (QAction *copyAction = m_actionCollection.action(QStringLiteral("copy"))) {
        copyAction->setVisible(!isTrash);
    }

    if (QAction *cutAction = m_actionCollection.action(QStringLiteral("cut"))) {
        cutAction->setVisible(!isTrash);
        cutAction->setEnabled(rootItem().isWritable());
    }

    if (QAction *restoreAction = m_actionCollection.action(QStringLiteral("restore"))) {
        restoreAction->setVisible(items.count() >= 1 && isTrash);
    }

    if (QAction *openWith = m_actionCollection.action(QStringLiteral("openWith"))) {
        openWith->setVisible(items.count() == 1 && !isTrash);
    }

    if (QAction *newFolder = m_actionCollection.action(QStringLiteral("newFolder"))) {
        newFolder->setVisible(!isTrash);
        newFolder->setEnabled(rootItem().isWritable());
    }

    if (QAction *newTextFile = m_actionCollection.action(QStringLiteral("newTextFile"))) {
        newTextFile->setVisible(!isTrash);
        newTextFile->setEnabled(rootItem().isWritable());
    }

    if (QAction *paste = m_actionCollection.action(QStringLiteral("paste"))) {
        bool enable = false;

        const QMimeData *mimeData = QApplication::clipboard()->mimeData();
        QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);

        if (!urls.isEmpty()) {
            if (!rootItem().isNull()) {
                enable = rootItem().isWritable();
            }
        }

        paste->setEnabled(enable);
        paste->setVisible(!isTrash);
    }

    if (QAction *rename = m_actionCollection.action(QStringLiteral("rename"))) {
        rename->setEnabled(itemProperties.supportsMoving());
        rename->setVisible(!isTrash && !Fm::isFixedFolder(items.first().url()));
    }

    if (QAction *trash = m_actionCollection.action("trash")) {
        trash->setEnabled(itemProperties.supportsMoving());
        trash->setVisible(!isTrash);
    }

    if (QAction *emptyTrash = m_actionCollection.action("emptyTrash")) {
        emptyTrash->setVisible(isTrash && rowCount() > 0);
    }

    if (QAction *del = m_actionCollection.action(QStringLiteral("del"))) {
        del->setVisible(isTrash && itemProperties.supportsDeleting());
    }

    if (QAction *terminal = m_actionCollection.action("terminal")) {
        terminal->setVisible(items.size() == 1 && items.first().isDir() && !isTrash);
    }

    if (QAction *terminal = m_actionCollection.action("setWallpaper")) {
        terminal->setVisible(items.size() == 1 &&
                             !isTrash &&
                             supportSetAsWallpaper(items.first().mimetype()));
    }

    if (QAction *terminal = m_actionCollection.action("wallpaperLogin")) {
        terminal->setVisible(items.size() == 1 &&
                             !isTrash &&
                             supportSetAsWallpaper(items.first().mimetype()));
    }

    if (QAction *properties = m_actionCollection.action("properties")) {
        properties->setVisible(!isTrash);
    }

    if (QAction *showHidden = m_actionCollection.action("showHidden")) {
        showHidden->setVisible(!isTrash);
        showHidden->setCheckable(true);
        showHidden->setChecked(m_showHiddenFiles);
    }

    if (QAction *openInNewWindow = m_actionCollection.action("openInNewWindow")) {
        openInNewWindow->setVisible(hasDir && !isTrash);
    }

    if (QAction *iconSizeMenu = m_actionCollection.action("iconSizeMenu")) {
        iconSizeMenu->setVisible(m_isDesktop);
    }

    if (QAction *viewMenu = m_actionCollection.action("viewMenu")) {
        viewMenu->setVisible(m_isDesktop);
    }

    if (QAction *refresh = m_actionCollection.action("refresh")) {
        refresh->setVisible(!isTrash);
    }
}

void FolderModel::addDragImage(QDrag *drag, int x, int y)
{
    if (!drag || m_dragImages.isEmpty())
        return;

    QRegion region;

    foreach (DragImage *image, m_dragImages) {
        image->blank = isBlank(image->row);
        image->rect.translate(-m_dragHotSpotScrollOffset.x(), -m_dragHotSpotScrollOffset.y());
        if (!image->blank && !image->image.isNull()) {
            region = region.united(image->rect);
        }
    }

    QRect rect = region.boundingRect();
    QPoint offset = rect.topLeft();
    rect.translate(-offset.x(), -offset.y());

    QImage dragImage(rect.size(), QImage::Format_RGBA8888);
    dragImage.fill(Qt::transparent);

    QPainter painter(&dragImage);
    QPoint pos;

    foreach (DragImage *image, m_dragImages) {
        if (!image->blank && !image->image.isNull()) {
            pos = image->rect.translated(-offset.x(), -offset.y()).topLeft();
            image->cursorOffset.setX(pos.x() - (x - offset.x()));
            image->cursorOffset.setY(pos.y() - (y - offset.y()));

            painter.drawImage(pos, image->image);
        }

        // FIXME HACK: Operate on copy.
        image->rect.translate(m_dragHotSpotScrollOffset.x(), m_dragHotSpotScrollOffset.y());
    }

    drag->setPixmap(QPixmap::fromImage(dragImage));
    drag->setHotSpot(QPoint(x - offset.x(), y - offset.y()));
}

void FolderModel::keyDeleteForever()
{
    if (!m_selectionModel->hasSelection())
        return;

    DeleteDialog *dlg = new DeleteDialog(selectedUrls());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}
