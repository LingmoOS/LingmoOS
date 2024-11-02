/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-item-model.h"
#include "file-item.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-infos-job.h"
#include "file-meta-info.h"

#include "file-operation-manager.h"
#include "file-move-operation.h"
#include "file-copy-operation.h"
#include "file-operation-helper.h"

#include "file-utils.h"

#include "thumbnail-manager.h"
#include "global-settings.h"

#include "file-operation-utils.h"

#include "emblem-provider.h"
#include "sound-effect.h"
#ifdef LINGMO_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif

#include <QIcon>
#include <QMimeData>
#include <QUrl>

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QGSettings>

using namespace Peony;
#ifdef LINGMO_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

FileItemModel::FileItemModel(QObject *parent) : QAbstractItemModel (parent)
{
    setPositiveResponse(true);

    m_fileManagerThread = new FileManagerThread();
    m_fileManagerThread->moveToThread(m_fileManagerThread);
    connect(this, &FileItemModel::setUrisForBatchQueryInfos, this, [=] (const QStringList& uris, int operateType, FileItem *parentItem) {
        auto infos = FileInfo::fromUris(uris);
        m_infosJob = new FileInfosJob(infos, parentItem);
        m_infosJob->connect(m_root_item, &FileItem::cancelFindChildren, m_infosJob, &FileInfosJob::batchCancel);
        m_infosJob->connect(this, &FileItemModel::cancelBatchQuery, m_infosJob, &FileInfosJob::batchCancel);
        Q_EMIT m_fileManagerThread->setParamForBatchQueryInfos(m_infosJob, operateType, parentItem);
    });
    m_fileManagerThread->start();

    connect(EmblemProviderManager::getInstance(), &EmblemProviderManager::requestUpdateFile, this, &FileItemModel::updated);
    connect(EmblemProviderManager::getInstance(), &EmblemProviderManager::requestUpdateAllFiles, this, &FileItemModel::updated);
    connect(EmblemProviderManager::getInstance(), &EmblemProviderManager::visibleChanged, this, &FileItemModel::updated);

    auto settings = GlobalSettings::getInstance();
    m_showFileExtension = settings->isExist(SHOW_FILE_EXTENSION)? settings->getValue(SHOW_FILE_EXTENSION).toBool(): true;
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (SHOW_FILE_EXTENSION == key) {
            m_showFileExtension= GlobalSettings::getInstance()->getValue(key).toBool();
            beginResetModel();
            endResetModel();
        }
    });
}

FileItemModel::~FileItemModel()
{
    qDebug()<<"~FileItemModel";
    disconnect();
    if (m_root_item)
        delete m_root_item;

    if(m_fileManagerThread){
        m_fileManagerThread->quit();
        m_fileManagerThread->wait();
        // 无法使用deleteLater()删除，因为此线程已经被移入自身，而且已经退出无法处理事件
        delete m_fileManagerThread;
    }
}

const QString FileItemModel::getRootUri()
{
    if (!m_root_item)
        return nullptr;
    return m_root_item->uri();
}

void FileItemModel::setRootUri(const QString &uri)
{
    if (uri.isNull()) {
        setRootUri("file:///");
        m_root_uri = "file:///";
        return;
    }
    m_root_uri = uri;
    auto info = FileInfo::fromUri(uri);
    auto item = new FileItem(info, nullptr, this, this);
    setRootItem(item);
}

void FileItemModel::setRootItem(FileItem *item)
{
    beginResetModel();
    m_root_item->deleteLater();

    m_root_item = item;

    m_root_item->connectFunc();
    if(m_infosJob){
        Q_EMIT cancelBatchQuery();
    }

    m_root_item->findChildrenAsync();

    endResetModel();
}

QModelIndex FileItemModel::index(int row, int column, const QModelIndex &parent) const
{
    //root children
    if (!parent.isValid()) {
        if (row < 0 || row > m_root_item->m_children->count()-1)
            return QModelIndex();
        return createIndex(row, column, m_root_item->m_children->at(row));
    }

    FileItem *item = static_cast<FileItem*>(parent.internalPointer());
    if (row < 0 || row > item->m_children->count()-1)
        return QModelIndex();
    return createIndex(row, column, item->m_children->at(row));
}

FileItem *FileItemModel::itemFromIndex(const QModelIndex &index) const
{
    return static_cast<FileItem*>(index.internalPointer());
}

QModelIndex FileItemModel::firstColumnIndex(FileItem *item)
{
    //root children
    if (item->m_parent == nullptr) {
        int index = m_root_item->m_children->indexOf(item);
        if (index == -1) {
            return QModelIndex();
        } else {
            return createIndex(index, 0, item);
        }
    } else {
        //has parent item
        int index = item->m_parent->m_children->indexOf(item);
        if (index == -1) {
            return QModelIndex();
        } else {
            return createIndex(index, 0, item);
        }
    }
}

QModelIndex FileItemModel::lastColumnIndex(FileItem *item)
{
    if (!item->m_parent) {
        int index = m_root_item->m_children->indexOf(item);
        if (index == -1) {
            //qDebug()<< "item uri:" << item->uri()<< "is not in the QVector";
            return QModelIndex();
        } else {
            //qDebug() << "item uri:" << item->uri() << "is at index" << index;
            return createIndex(index, Other, item);
        }
    } else {
        //has parent item
        int index = item->m_parent->m_children->indexOf(item);
        if (index == -1) {
            //qDebug()<< "item uri:" << item->uri()<< "is not in the QVector";
            return QModelIndex();
        } else {
            //qDebug() << "item uri:" << item->uri() << "is at index" << index;
            return createIndex(index, Other, item);
        }
    }
}

const QModelIndex FileItemModel::indexFromUri(const QString &uri)
{
    //FIXME: support recursively finding?
    if(m_root_item->m_uri_item_hash.contains(uri)) {
        auto child = m_root_item->m_uri_item_hash[uri];
        return indexFromItemAndUri(child, uri);
    }else if(m_root_item->m_uri_item_hash.contains(FileUtils::getEncodedUri(uri))){/* 中文编码问题 */
        QString encodedUri = FileUtils::getEncodedUri(uri);
        auto child = m_root_item->m_uri_item_hash[encodedUri];
        return indexFromItemAndUri(child, encodedUri);
    }else if(m_root_item->m_uri_item_hash.contains(FileUtils::urlDecode(uri))){/* 中文编码问题 */
            QString decodedUri = FileUtils::urlDecode(uri);
            auto child = m_root_item->m_uri_item_hash[decodedUri];
            return indexFromItemAndUri(child, decodedUri);
    }

    return QModelIndex();
}

QModelIndex FileItemModel::parent(const QModelIndex &child) const
{
    FileItem *childItem = static_cast<FileItem*>(child.internalPointer());
    //root children
    if (childItem->m_parent == nullptr)
        return QModelIndex();
    return childItem->m_parent->firstColumnIndex();
}

int FileItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_root_uri == "trash:///") {
        return FileSize + 2;
    }
    return FileSize+1;
}

int FileItemModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (!m_root_item) {
            return 0;
        }
        return m_root_item->m_children->count();
    }
    FileItem *parent_item = static_cast<FileItem*>(parent.internalPointer());
    return parent_item->m_children->count();
}

QVariant FileItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    FileItem *item = static_cast<FileItem*>(index.internalPointer());

    // we have to add uri role to every valid index, so that we can ensure
    // that we can open the file/directory correctly.
    if (role == FileItemModel::UriRole)
        return QVariant(item->uri());

    //qDebug()<<"data:" <<item->m_info->uri() << index.column();
    switch (index.column()) {
    case FileName: {
        switch (role) {
        case Qt::TextAlignmentRole: {
            return QVariant(Qt::AlignHCenter | Qt::AlignBaseline);
        }
        case Qt::DisplayRole: {
            //fix bug#53504, desktop files not show same name issue
            QString displayName = item->m_info->displayName();
            if (item->m_info->isDesktopFile())
            {
                displayName = FileUtils::handleDesktopFileName(item->m_info->uri(), item->m_info->displayName());
                return QVariant(displayName);
            }
            /* story#8359 【文件管理器】手动开启关闭文件拓展名 */
            if(!m_showFileExtension){
                if (item->m_info->isDir()||(!displayName.contains("."))) {
                    return QVariant(displayName);
                }
                return QVariant(FileUtils::getBaseNameOfFile(displayName));
            }else
                return QVariant(displayName);
        }
        case Qt::DecorationRole: {
            auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(item->m_info->uri());
            if (!thumbnail.isNull()) {
                return thumbnail;
            }
            QIcon icon = QIcon::fromTheme(item->m_info->iconName(), QIcon::fromTheme("unknown"));
            return QVariant(icon);
        }
        case Qt::ToolTipRole: {
            //fix bug#53504, desktop files not show same name issue
            if (item->m_info->isDesktopFile())
            {
                auto displayName = FileUtils::handleDesktopFileName(item->m_info->uri(), item->m_info->displayName());
                return QVariant(displayName);
            }
            return QVariant(item->m_info->displayName());
        }
        case Qt::UserRole + 1: {
            return item->m_info->displayName();
        }
        default:
            return QVariant();
        }
    }
    case ModifiedDate: {
        switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
            //trash files show delete Date
            if (m_root_uri.startsWith("trash://") && !item->m_info->deletionDate().isNull()) {
//                QDateTime deleteTime = QDateTime::fromMSecsSinceEpoch(item->m_info->deletionTime (), Qt::LocalTime);
//                QString format = GlobalSettings::getInstance()->getSystemTimeFormat();
                //use sdk interface to get time format
                return QVariant(item->m_info->deletionDate());
            }
            if (Peony::GlobalSettings::getInstance()->getShowCreateTime()) {
                return QVariant(item->m_info->createDate());
            }

            return QVariant(item->m_info->modifiedDate());
        default:
            return QVariant();
        }
    }
    case FileType:
        switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            if (item->m_info->isSymbolLink()) {
                return QVariant(tr("Symbol Link, ") + item->m_info->fileType());
            }
            return QVariant(item->m_info->fileType());
        }
        default:
            return QVariant();
        }
    case FileSize: {
        switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            if (item->hasChildren()) {
                if (item->m_expanded) {
                    return QVariant(QString::number(item->m_children->count()) + tr("child(ren)"));
                }
                return QVariant();
            }
            return QVariant(item->m_info->fileSize());
        }
        default:
            return QVariant();
        }
    }
    case TrashOriginPath: {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
            QString originPath = item->m_info->property("orig-path").toString();
            if (originPath.isEmpty()) {
                auto targetInfo = FileInfo::fromUri(item->m_info->targetUri());
                if (targetInfo->isEmptyInfo()) {
                    FileInfoJob j(targetInfo);
                    j.querySync();
                    originPath = FileMetaInfo::fromUri(targetInfo->uri())->getMetaInfoString("orig-path");
                    item->m_info->setProperty("orig-path", originPath);
                }
                return originPath;
            }
            return originPath;
            break;
        }
        default:
            break;
        }
    }
    default:
        return QVariant();
    }
}

QVariant FileItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        //qDebug() <<"headerData:" <<section;
        switch (section) {
        case FileName:
            return tr("File Name");
        case ModifiedDate:
            //trash files show delete Date
            if (m_root_uri.startsWith("trash:///"))
                return tr("Delete Date");
            if (Peony::GlobalSettings::getInstance()->getValue(SHOW_CREATE_TIME).toBool()) {
                return tr("Create Date");
            }

            return tr("Modified Date");
        case FileType:
            return tr("File Type");
        case FileSize:
            return tr("File Size");
        case TrashOriginPath:
            return tr("Original Path");

        default:
            return QVariant();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool FileItemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    FileItem *parent_item = static_cast<FileItem*>(parent.internalPointer());
    if (parent_item->hasChildren() && m_can_expand)
        return true;
    return false;
}

Qt::ItemFlags FileItemModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);

        auto item = itemFromIndex(index);
        if (item->m_info->isDir() && item->m_info->canWrite()) {
            flags |= Qt::ItemIsDropEnabled;
        }
        if (index.column() == FileName) {
            flags |= Qt::ItemIsDragEnabled;
            flags |= Qt::ItemIsEditable;
        }
        // to make the applications disable
        if(item->m_info->canExecute()&&item->m_info->isExecDisable()) {
            flags &= ~Qt::ItemIsEnabled;
            flags |= Qt::ItemIsSelectable;
        }
        return flags;
    } else {
        if (m_root_item) {
            if (m_root_item->m_info->canWrite()) {
                return Qt::ItemIsDropEnabled;
            } else if(m_root_item->m_info.get()->fileSystemType().contains("udf")) {
                return Qt::ItemIsDropEnabled;
            }
        }
        return Qt::ItemIsDropEnabled;
    }
}

bool FileItemModel::canFetchMore(const QModelIndex &parent) const
{
    //qDebug()<<"canFetchMore";
    if (!parent.isValid())
        return true;
    FileItem *parent_item = static_cast<FileItem*>(parent.internalPointer());
    if (!parent_item->m_expanded) {
        return true;
    }
    return false;
}

void FileItemModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    //do not fetch more here
}

bool FileItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool FileItemModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool FileItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

bool FileItemModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

void FileItemModel::onFoundChildren(const QModelIndex &parent)
{
    if (!parent.isValid()) {
        return;
    }
    FileItem *parentItem = static_cast<FileItem*>(parent.internalPointer());
    beginInsertRows(parent, 0, parentItem->m_children->count() - 1);
    endInsertRows();
}

void FileItemModel::onItemAdded(FileItem *item)
{
    if (!item->m_parent)
        insertRow(item->firstColumnIndex().row());
    insertRow(item->firstColumnIndex().row(), item->m_parent->firstColumnIndex());
}

void FileItemModel::onItemRemoved(FileItem *item)
{
    if (!item->m_parent)
        removeRow(item->firstColumnIndex().row());
    removeRow(item->firstColumnIndex().row(), item->m_parent->firstColumnIndex());
}

void FileItemModel::cancelFindChildren()
{
    qDebug()<<"cancel";
    // try fix #164883, error cusor while searching
    m_root_item->setProperty("isCancelled", true);
    m_root_item->cancelFindChildren();
}

void FileItemModel::setRootIndex(const QModelIndex &index)
{
    //NOTE: if we use proxy model, we might get the wrong item from index.
    //add the new data role save the file's uri to resolve this problem.
    if (index.isValid()) {
        auto new_root_info = FileInfo::fromUri(index.data(UriRole).toString());
        auto new_root_item = new FileItem(new_root_info,
                                          nullptr,
                                          this);
        if (new_root_item->hasChildren()) {
            setRootItem(new_root_item);
        }
    }
}

void FileItemModel::updateCurrentFilesThumbnails()
{
    for (FileItem *child : *(m_root_item->m_children)) {
        auto uri = child->uri();
        ThumbnailManager::getInstance()->createThumbnail(uri, m_root_item->m_thumbnail_watcher);
    }
}

#include <QVariant>
QMimeData *FileItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = QAbstractItemModel::mimeData(indexes);
    //set urls data URLs correspond to the MIME type text/uri-list.
    QList<QUrl> urls;
    QStringList uris;
    QStringList encodedUris;
    for (auto index : indexes) {
        if (index.isValid() && index.column() == 0) { /* 仅处理有效的第一列索引 */
            QVariant var = data(index, FileItemModel::UriRole);
            auto uri = var.toString();
            QUrl url = uri;
            urls << url;
            uris << uri;
            encodedUris<<uri;
        }
    }
    mimeData->setUrls(urls);
    auto string = uris.join(" ");
    auto encodedString = encodedUris.join(" ");
    mimeData->setData("explor-qt/encoded-uris", encodedString.toUtf8());
    mimeData->setText(string);
    return mimeData;
}

Qt::DropActions FileItemModel::supportedDropActions() const
{
    //qDebug()<<"supportedDropActions";
    return Qt::MoveAction|Qt::CopyAction;
}

Qt::DropActions FileItemModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

bool FileItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug()<<"drop mime data";
    //judge the drop dest uri.
    QString destDirUri = nullptr;
    if (parent.isValid()) {
        QModelIndex child = index(row, column, parent);
        if (child.isValid()) {
            //unexpected drop.
            /*
            auto item = static_cast<FileItem*>(child.internalPointer());
            qDebug()<<item->m_info->uri();
            if (item->m_info->isDir()) {
                destDirUri = item->m_info->uri();
            }
            */
        } else {
            //drop on a folder item.
            auto parentItem = itemFromIndex(parent);
            destDirUri = parentItem->m_info->uri();
        }
    } else {
        //FIXME: for a mounted volume (for example, computer:///),
        //we have to set the dest dir uri as its mount point.
        //maybe i should do this when set model root item.
        destDirUri = m_root_item->m_info->uri();
    }

    //get real destDirUri
    auto targetUri = FileUtils::getTargetUri(destDirUri);
    if (!targetUri.isEmpty()) {
        destDirUri = targetUri;
    }

    //if destDirUri was not set, do not execute a drop.
    if (destDirUri.isNull()) {
        return false;
    }

    //if drag file to empty CD or DVD, is invalid operation, link to bug#129347
    //如果是拖拽文件到空光盘，操作无效，光盘只能刻录，不能直接写入文件
    if (destDirUri.startsWith("burn:///"))
        return false;

    auto info = Peony::FileInfo::fromUri(destDirUri);
    //qDebug() << "FileItemModel::dropMimeData:" <<info->isDir() <<info->type();
    //if (!FileUtils::getFileIsFolder(destDirUri))
    //fix drag file to folder symbolic fail issue
    if (! info->isDir() && ! destDirUri.startsWith("trash:///"))
        return false;

    //NOTE:
    //do not allow drop on it self.
    auto urls = data->urls();
    if (urls.isEmpty())
        return false;

    bool hasPeonyEncodedUris = false;
    if (data->hasFormat("explor-qt/encoded-uris")) {
        if (!data->data("explor-qt/encoded-uris").isEmpty()) {
            hasPeonyEncodedUris = true;
        }
    }

    QStringList srcUris;
    if (hasPeonyEncodedUris) {
        srcUris = QString(data->data("explor-qt/encoded-uris")).split(" ");
        for (QString uri : srcUris) {
            if (uri.startsWith("recent://"))
                srcUris.removeOne(uri);
        }
    } else {
        for (auto url : urls) {
            //can not drag file from recent
            if (url.url().startsWith("recent://"))
                return false;
            srcUris<<url.url();
        }
    }

    srcUris.removeDuplicates();
    if (srcUris.isEmpty())
        return false;

    //can not drag file to recent
    if (destDirUri.startsWith("recent://"))
        return false;

    if (srcUris.contains(destDirUri)) {
        return false;
    }

    //can not move StandardPath to any dir
    if (action == Qt::MoveAction && FileUtils::containsStandardPath(srcUris)) {
        return false;
    }


    bool bMoveFromSearchTab = false;
    if (data->hasFormat("explor-qt/is-search")) {
        bMoveFromSearchTab = QVariant(data->data("explor-qt/is-search")).toBool();
    }

    bool b_trash_item = false;
    for(auto path : srcUris)
    {
        if (path.contains("trash:///"))
        {
            b_trash_item = true;
            break;
        }
    }
    //drag from trash to another place, return false
    //comment to fix can not drag to copy trash file,link to bug#117741
//    if (b_trash_item && destDirUri != "trash:///")
//        return false;

    //fix drag file to trash issue, #42328
    if (destDirUri.startsWith("trash:///"))
    {
        // fix drag filesafe to trash error issue, #81938
        if(!(srcUris.first().startsWith("filesafe:///") &&
            (QString(srcUris.first()).remove("filesafe:///").indexOf("/") == -1))) {
            FileOperationUtils::trash(srcUris, true, bMoveFromSearchTab);
        }
        return true;
    }

    //fix drag trash file to other path is copy issue,link to bug#117741
    if (srcUris.first().startsWith("trash:///") && action == Qt::MoveAction){
        //not copy move, do target move to delete file in trash
        action = Qt::TargetMoveAction;
    }


    qDebug() << "dropMimeData:" <<action<<destDirUri;
    bool addHistory = true;
    //krme files can not move to other place, default set as copy action
    if (srcUris.first().startsWith("kmre:///") || srcUris.first().startsWith("kydroid:///"))
        action = Qt::CopyAction;

    //filesafe files can not move to other place, default set as copy action
    if (srcUris.first().startsWith("filesafe:///") && destDirUri.startsWith("favorite:///"))
        return false;
    if (srcUris.first().startsWith("filesafe:///"))
        action = Qt::CopyAction;

    auto op = FileOperationUtils::moveWithAction(srcUris, destDirUri, addHistory, action, bMoveFromSearchTab);
    connect(op, &FileOperation::operationFinished, this, [=](){
        auto opInfo = op->getOperationInfo();
        if (! opInfo->m_has_error){
            //Peony::SoundEffect::getInstance()->copyOrMoveSucceedMusic();
            //Task#152997, use sdk play sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
            kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
#endif
        }
        auto targetUris = opInfo.get()->dests();
        Q_EMIT this->selectRequest(targetUris);
//            auto selectionModel = new QItemSelectionModel(this);
//            selectionModel->clearSelection();
//            QTimer::singleShot(1000, selectionModel, [=](){
//                for (auto destUri : targetUris) {
//                    auto index = indexFromUri(destUri);
//                    selectionModel->select(index, QItemSelectionModel::Select);
//                }
//                selectionModel->deleteLater();
//            });
    }, Qt::BlockingQueuedConnection);

    //NOTE:
    //we have to handle the dnd with file operation, so do not
    //use QAbstractModel::dropMimeData() here;
    return true;
}

void FileItemModel::sendPathChangeRequest(const QString &destUri, const QString &sourceUri)
{
    Q_EMIT this->changePathRequest(destUri, sourceUri);
}

void FileItemModel::setShowFileExtensions(bool show)
{
    m_showFileExtension = show;
    GlobalSettings::getInstance()->setGSettingValue(SHOW_FILE_EXTENSION, show);
}

const QModelIndex FileItemModel::indexFromItemAndUri(FileItem *item, const QString &uri)
{
    if(!item)
        return QModelIndex();

    GFile *left = g_file_new_for_uri(item->uri().toUtf8().constData());
    GFile *right = g_file_new_for_uri(uri.toUtf8().constData());
    bool equal = g_file_equal(left, right);
    g_object_unref(left);
    g_object_unref(right);
    if (equal) {
        return item->firstColumnIndex();
    }
    return QModelIndex();
}

FileManagerThread::FileManagerThread():QThread(nullptr)
{
    connect(this, &FileManagerThread::setParamForBatchQueryInfos, this, &FileManagerThread::batchQueryFileInfos);
}

FileManagerThread::~FileManagerThread()
{
}

void FileManagerThread::batchQueryFileInfos(FileInfosJob *infosJob,  /*FileItemModel::OperateType*/int operateType, FileItem *parentItem)
{
    auto retFileInfos = infosJob->batchQuerySync();
    if(infosJob){
        infosJob->deleteLater();
    }
    Q_EMIT finishQueryFileInfos(retFileInfos, operateType, parentItem);

}

void FileManagerThread::run()
{
    exec();
}
