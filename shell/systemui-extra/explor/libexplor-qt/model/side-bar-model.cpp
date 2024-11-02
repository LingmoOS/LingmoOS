/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "side-bar-model.h"
#include "side-bar-favorite-item.h"
#include "side-bar-personal-item.h"
#include "side-bar-file-system-item.h"
#include "side-bar-separator-item.h"
#include "side-bar-net-work-item.h"

#include "file-info.h"
#include "file-info-job.h"

#include "bookmark-manager.h"
#include "file-operation-utils.h"

#include "vfs-plugin-manager.h"
#include "side-bar-vfs-item.h"

#include "side-bar-single-item.h"
#include "file-utils.h"

#include <QIcon>
#include <QMimeData>
#include <QUrl>

#include <QDebug>

using namespace Peony;

SideBarModel::SideBarModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    beginResetModel();

    m_root_children = new QVector<SideBarAbstractItem*>();

    auto vfsMgr = VFSPluginManager::getInstance();
    auto plugins = vfsMgr->registeredPlugins();
    for (auto plugin : plugins) {
        if (plugin->uriScheme().contains("kydroid://") || plugin->uriScheme().contains("kmre://")) {
            continue;
        }
        if (plugin->holdInSideBar()) {
            m_root_children->append(new SideBarVFSItem(plugin->uriScheme() + "/", nullptr, this));
        }
    }

//    SideBarSeparatorItem *separator1 = new SideBarSeparatorItem(SideBarSeparatorItem::Large, nullptr, this, this);
//    m_root_children->append(separator1);

    SideBarFavoriteItem *favorite_root_item = new SideBarFavoriteItem(nullptr, nullptr, this);
    m_root_children->append(favorite_root_item);
    //favorite_root_item->findChildren();

//    SideBarSeparatorItem *separator2 = new SideBarSeparatorItem(SideBarSeparatorItem::Small, nullptr, this, this);
//    m_root_children->append(separator2);

//    if (FileUtils::isFileExsit("file:///data/usershare")) {
//        SideBarSingleItem *userShareItem = new SideBarSingleItem("file:///data/usershare", nullptr, tr("Shared Data"), this);
//        m_root_children->append(userShareItem);
//    }

    SideBarPersonalItem *personal_root_item = new SideBarPersonalItem(nullptr, nullptr, this);
    m_root_children->append(personal_root_item);
    //personal_root_item->findChildren();

//    SideBarSeparatorItem *separator3 = new SideBarSeparatorItem(SideBarSeparatorItem::Small, nullptr, this, this);
//    m_root_children->append(separator3);

    SideBarFileSystemItem *computerItem = new SideBarFileSystemItem(nullptr,nullptr,
            nullptr,
            this);
    m_root_children->append(computerItem);
    //computerItem->findChildren();

//    SideBarSingleItem *networkItem = new SideBarSingleItem("network:///", "network-workgroup-symbolic", tr("Network"), this);
    SideBarNetWorkItem *networkItem = new SideBarNetWorkItem("network:///", "network-workgroup-symbolic",
                                                             tr("Network"), nullptr, this);
    m_root_children->append(networkItem);

    endResetModel();

    //empty-file separator
    connect(this, &SideBarModel::indexUpdated, this, &SideBarModel::onIndexUpdated);
}

SideBarModel::~SideBarModel()
{
    for (auto child : *m_root_children) {
        delete child;
    }
    m_root_children->clear();
    delete m_root_children;
}

QModelIndex SideBarModel::firstColumnIndex(SideBarAbstractItem *item)
{
    if (item->parent() != nullptr) {
        return createIndex(item->parent()->m_children->indexOf(item), 0, item);
    } else {
        return createIndex(m_root_children->indexOf(item), 0, item);
    }
}

QModelIndex SideBarModel::lastColumnIndex(SideBarAbstractItem *item)
{
    if (item->parent() != nullptr) {
        createIndex(item->parent()->m_children->indexOf(item), 1, item);
    } else {
        for (auto child : *m_root_children) {
            if (item->type() == child->type()) {
                return createIndex(m_root_children->indexOf(child), 1, item);
            }
        }
    }
    return QModelIndex();
}

SideBarAbstractItem *SideBarModel::itemFromIndex(const QModelIndex &index)
{
    return static_cast<SideBarAbstractItem*>(index.internalPointer());
}

QVariant SideBarModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

bool SideBarModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return false;
}

QModelIndex SideBarModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column, m_root_children->at(row));
    }
    SideBarAbstractItem *parentItem = static_cast<SideBarAbstractItem*>(parent.internalPointer());
    if (parentItem->m_children->count() > row) {
        return createIndex(row, column, parentItem->m_children->at(row));
    }
    return QModelIndex();
}

QModelIndex SideBarModel::parent(const QModelIndex &index) const
{
    SideBarAbstractItem *item = static_cast<SideBarAbstractItem*>(index.internalPointer());
    //qDebug()<<item->uri();
    if (!item)
        return QModelIndex();
    if (item->parent())
        return item->parent()->firstColumnIndex();

    return QModelIndex();
}

int SideBarModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_root_children->count();

    SideBarAbstractItem *parentItem = static_cast<SideBarAbstractItem*>(parent.internalPointer());
    //qDebug()<<parentItem->uri();
    return parentItem->m_children->count();
}

int SideBarModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

bool SideBarModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;

    SideBarAbstractItem *parentItem = static_cast<SideBarAbstractItem*>(parent.internalPointer());
    return parentItem->hasChildren();
}

bool SideBarModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return true;
}

void SideBarModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
}

QVariant SideBarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    SideBarAbstractItem *item = static_cast<SideBarAbstractItem*>(index.internalPointer());

    if (index.column() == 0 && !index.parent().isValid()) {
        if(role == Qt::DecorationRole) {
            return QVariant();
        }
    }

    if (index.column() == 1) {
        if(role == Qt::DecorationRole){
            bool unmountAble,ejectAble;
            unmountAble = item->isUnmountable();
            ejectAble = item->isEjectable()||item->isStopable();
            if(unmountAble || ejectAble){
                if(item->isMounted())
                    return QVariant(QIcon::fromTheme("media-eject-symbolic"));
                else
                    return QVariant();
            }else
                return QVariant();
        }else
            return QVariant();
    }

    switch (role) {
    case Qt::DecorationRole:{
        QString iconName = item->iconName();
        if(item->type() == SideBarAbstractItem::FileSystemItem
                && item->getDevice().startsWith("/dev/sr")
                && (iconName.startsWith("media-optical"))){/* 规范光盘图标，与竞品对比，光盘图标使用的是"media-optical-symbolic",linkto bug#174770 */
            iconName = "media-optical-symbolic";
        }
        if(!iconName.endsWith("-symbolic")){
            iconName += "-symbolic";
        }
        //qDebug()<<"print side bar icon name, uri"<<FileUtils::urlEncode(item->uri())<<" icon name:"<<iconName;
        return QIcon::fromTheme(iconName, QIcon::fromTheme(item->iconName()));
    }
    case Qt::DisplayRole:
        return item->displayName();
    case Qt::ToolTipRole:
        return item->displayName();
    case Qt::UserRole:
        return item->uri();
    default:
        break;
    }
    return QVariant();
}

bool SideBarModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        Q_EMIT dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags SideBarModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled; // FIXME: Implement me!
}

bool SideBarModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool SideBarModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool SideBarModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count == 0)
        return true;
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

bool SideBarModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

void SideBarModel::onIndexUpdated(const QModelIndex &index)
{
    auto item = itemFromIndex(index);
    //qDebug()<<item->m_children->count();
    bool isEmpty = true;
    for (auto child : *item->m_children) {
        auto info = FileInfo::fromUri(child->uri());
        FileInfoJob job(child->uri());
        job.querySync();
        if (!info->displayName().startsWith(".") && (info->isDir() || info->isVolume())||item->uri()=="computer:///")
            isEmpty = false;
        if (child->type() == SideBarAbstractItem::SeparatorItem) {
            removeRows(item->m_children->indexOf(child), 1, index);
            item->m_children->removeOne(child);
            qDebug()<<"separator"<<item->m_children->count();
        }
    }
    if (isEmpty) {
        auto separator = new SideBarSeparatorItem(SideBarSeparatorItem::EmptyFile, item, this);
        item->m_children->append(separator);
        insertRows(item->m_children->count() - 1, 1, index);
    }
}

bool SideBarModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    auto urls = data->urls();

    QStringList srcUris;
    if (data->hasFormat("explor-qt/encoded-uris")) {
        srcUris = data->text().split(" ");
        for (QString uri : srcUris) {
            if (uri.startsWith("recent://") || uri.startsWith("filesafe://"))
                srcUris.removeOne(uri);
        }
    } else {
        for (auto url : urls) {
            //can not drag file from recent
            if (url.url().startsWith("recent://") || url.url().startsWith("filesafe://"))
                return false;
            srcUris<<url.url();
        }
    }
    srcUris.removeDuplicates();

    if (srcUris.isEmpty()) {
        return false;
    }

    auto item = this->itemFromIndex(parent);
    qDebug()<<"SideBarModel::dropMimeData:" <<action<<row<<column<<parent.data();
    if (!parent.isValid()) {

        auto bookmark = BookMarkManager::getInstance();
        if (bookmark->isLoaded()) {
            for (auto url : srcUris) {
                //FIXME: replace BLOCKING api in ui thread.
                auto info = FileInfo::fromUri(url);
                if (info->displayName().isNull()) {
                    FileInfoJob j(info);
                    j.querySync();
                }
                if (info->isDir()) {
                    bookmark->addBookMark(url);
                }
            }
        }
        return true;
    }

    switch (item->type()) {
    case SideBarAbstractItem::SeparatorItem:
    case SideBarAbstractItem::FavoriteItem:
    //drag to sider bar all as file move operation
//    {
//        auto bookmark = BookMarkManager::getInstance();
//        if (bookmark->isLoaded()) {
//            for (auto url : data->urls()) {
//                auto info = FileInfo::fromUri(url.toDisplayString(), false);
//                if (info->displayName().isNull()) {
//                    FileInfoJob j(info);
//                    j.querySync();
//                }
//                if (info->isDir()) {
//                    bookmark->addBookMark(url.url());
//                }
//            }
//        }
//        break;
//    }
    case SideBarAbstractItem::PersonalItem:
    case SideBarAbstractItem::FileSystemItem: {
        QStringList uris = srcUris;

        //can not drag file to recent
        if (item->uri().startsWith("recent://"))
            return false;

        for(auto uri : uris)
        {
            if (uri.startsWith("trash://"))
                return false;
            //can not drag file from recent
            if (uri.startsWith("recent://"))
                return false;

            //如果源文件路径是filesafe保护箱下的文件只支持复制，不支持移动；
            if(uri.startsWith("filesafe:///")) {
                action = Qt::CopyAction;
            }
        }

        if (action == Qt::MoveAction)
        {
            //can not move StandardPath to any dir
            if (FileUtils::containsStandardPath(srcUris)) {
                return false;
            }
            bool bMoveFromSearchTab = false;
            if (data->hasFormat("explor-qt/is-search")) {
                bMoveFromSearchTab = QVariant(data->data("explor-qt/is-search")).toBool();
            }
            FileOperationUtils::moveWithAction(uris, item->uri(), true, Qt::MoveAction, bMoveFromSearchTab);
            //qDebug() << "sideBarModel moveOp";
        }
        else if (action == Qt::CopyAction)
        {
            FileOperationUtils::copy(uris, item->uri(), true);
            //qDebug() << "sideBarModel CopyOp";
        }
        break;
    }
    }
    return true;
}

Qt::DropActions SideBarModel::supportedDropActions() const
{
    return Qt::MoveAction|Qt::CopyAction;
    return Qt::MoveAction|Qt::CopyAction|Qt::LinkAction;
}

Qt::DropActions SideBarModel::supportedDragActions() const
{
    return Qt::MoveAction;
}
