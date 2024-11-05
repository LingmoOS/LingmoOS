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

#include "intel-side-bar-model.h"
#include "intel-side-bar-favorite-item.h"
#include "intel-side-bar-personal-item.h"
#include "intel-side-bar-file-system-item.h"
#include "intel-side-bar-separator-item.h"
#include "intel-side-bar-cloud-item.h"

#include "file-info.h"
#include "file-info-job.h"

#include "bookmark-manager.h"
#include "file-operation-utils.h"

#include "vfs-plugin-manager.h"
#include "intel-side-bar-vfs-item.h"

#include <QIcon>
#include <QMimeData>
#include <QUrl>

#include <QDebug>

using namespace Peony::Intel;

SideBarModel::SideBarModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    beginResetModel();

    m_root_children = new QVector<SideBarAbstractItem*>();

    auto vfsMgr = VFSPluginManager::getInstance();
    auto plugins = vfsMgr->registeredPlugins();
    for (auto plugin : plugins) {
        if (plugin->holdInSideBar())
            m_root_children->append(new SideBarVFSItem(plugin, this));
    }

//    SideBarSeparatorItem *separator1 = new SideBarSeparatorItem(SideBarSeparatorItem::Large, nullptr, this, this);
//    m_root_children->append(separator1);

    SideBarFavoriteItem *favorite_root_item = new SideBarFavoriteItem(nullptr, nullptr, this);
    m_root_children->append(favorite_root_item);
    //favorite_root_item->findChildren();

//    SideBarSeparatorItem *separator2 = new SideBarSeparatorItem(SideBarSeparatorItem::Small, nullptr, this, this);
//    m_root_children->append(separator2);

    SideBarPersonalItem *personal_root_item = new SideBarPersonalItem(nullptr, nullptr, this);
    m_root_children->append(personal_root_item);
    //personal_root_item->findChildren();

//    SideBarSeparatorItem *separator3 = new SideBarSeparatorItem(SideBarSeparatorItem::Small, nullptr, this, this);
//    m_root_children->append(separator3);

    //! \note Temporarily hidden cloud
//    SideBarCloudItem *cloudItem = new SideBarCloudItem(nullptr, nullptr, this);
//    m_root_children->append(cloudItem);

    SideBarFileSystemItem *computerItem = new SideBarFileSystemItem(nullptr, nullptr, this);
    m_root_children->append(computerItem);
    //computerItem->findChildren();


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
    return 1;
}

bool SideBarModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    // to make item in side bar can expand only one time
    if (parent.parent().isValid())
        return false;

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

    //! Delete the second column to fit the topic
//    if (index.column() == 1) {
//        if(role == Qt::DecorationRole){
//            bool unmountAble,ejectAble;
//            unmountAble = item->isMountable();
//            ejectAble = item->isEjectable();
//            if(unmountAble && ejectAble)
//                return QVariant(QIcon::fromTheme("media-eject"));
//            else if(unmountAble){
//                if(item->isMounted())
//                    return QVariant(QIcon::fromTheme("media-eject"));
//                else
//                    return QVariant();
//            }else
//                return QVariant();
//        }else
//            return QVariant();
//    }

    switch (role) {
    case Qt::DecorationRole:
        return QIcon::fromTheme(item->iconName() + "-symbolic", QIcon::fromTheme(item->iconName()));
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
        if (!info->displayName().startsWith(".") && (info->isDir() || info->isVolume()))
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
    if (data->urls().isEmpty())
        return false;

    auto item = this->itemFromIndex(parent);
    qDebug()<<"SideBarModel::dropMimeData:" <<action<<row<<column<<parent.data();
    if (!parent.isValid()) {

        auto bookmark = BookMarkManager::getInstance();
        if (bookmark->isLoaded()) {
            for (auto url : data->urls()) {
                auto info = FileInfo::fromUri(url.toDisplayString());
                if (info->displayName().isNull()) {
                    FileInfoJob j(info);
                    j.querySync();
                }
                if (info->isDir()) {
                    bookmark->addBookMark(url.url());
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
        QStringList uris;
        for (auto url : data->urls()) {
            uris<<url.url();
        }

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
        }
        FileOperationUtils::move(uris, item->uri(), true, true);
        break;
    }
    }
    return true;
}

Qt::DropActions SideBarModel::supportedDropActions() const
{
    return Qt::MoveAction|Qt::CopyAction|Qt::LinkAction;
}
