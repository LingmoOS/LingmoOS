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

#include "side-bar-proxy-filter-sort-model.h"
#include "side-bar-abstract-item.h"
#include "side-bar-model.h"

#include "file-utils.h"
#include "file-info.h"
#include "file-info-job.h"

#include "global-settings.h"

#include <QUrl>
#include <QDir>
#include <QDebug>

using namespace Peony;

SideBarProxyFilterSortModel::SideBarProxyFilterSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    m_locale = QLocale(QLocale::system().name());
    m_comparer = QCollator(m_locale);
    m_comparer.setNumericMode(true);

    m_show_hidden = GlobalSettings::getInstance()->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == SHOW_HIDDEN_PREFERENCE) {
            m_show_hidden = GlobalSettings::getInstance()->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
            invalidateFilter();
        }
    });
}

bool SideBarProxyFilterSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto item = static_cast<SideBarAbstractItem*>(index.internalPointer());

    if (item->type() != SideBarAbstractItem::SeparatorItem) {
        if (item->displayName().isNull() && (item->type() == SideBarAbstractItem::FileSystemItem || item->type() == SideBarAbstractItem::VFSItem))
            return false;

        //not exist path filter
        if (item->type() == SideBarAbstractItem::FavoriteItem && ! item->uri().isEmpty())
        {
            QDir dir(QUrl(item->uri()).path());
            if (! dir.exists())
                return false;
        }
    }

    if (item->type() == SideBarAbstractItem::NetWorkItem) {
        if (item->uri().isEmpty()) {
            return false;
        }
    }
    //comment to fix bug 41426, user add .config file to bookmark for convinient accesss
//    if (item) {
//        if (!item->displayName().isEmpty()) {
//            if (QString(item->displayName().at(0)) == ".") {
//                return false;
//            }
//        }
//    }
    if (item->type() == SideBarAbstractItem::FileSystemItem) {
        if (sourceParent.data(Qt::UserRole).toString() == "computer:///") {
           return item->filterShowRow();
        }

        /*!
          所有的volumeitem必须要有一个对应的uri才能显示，这个uri或者是mountpoint（已挂载的）或者是computer:///xxx（未挂载的），
          否则会出现drive和volume同时存在的bug，参考#90081
          */
        if (item->uri().isEmpty()) {
            return false;
        }

        if (!m_show_hidden) {
            return !item->displayName().startsWith(".");
        }

    }

    return true;
}

bool SideBarProxyFilterSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    //qDebug()<<"less than";
    /* 第一层不排序 */
    if(!left.parent().isValid() && !right.parent().isValid())
        return sortOrder() == Qt::AscendingOrder? true: false;

    if (!(left.isValid() && right.isValid())) {
        return QSortFilterProxyModel::lessThan(left, right);
    }
    auto leftItem = static_cast<SideBarAbstractItem*>(left.internalPointer());
    auto rightItem = static_cast<SideBarAbstractItem*>(right.internalPointer());
    bool isVFSorFileSystemItem = false;
    bool isVFSItem = false;
    if (leftItem->type() == SideBarAbstractItem::FileSystemItem && rightItem->type() == SideBarAbstractItem::FileSystemItem) {
        isVFSorFileSystemItem = true;
    }
    if (leftItem->type() == SideBarAbstractItem::VFSItem && rightItem->type() == SideBarAbstractItem::VFSItem) {
        isVFSorFileSystemItem = true;
        isVFSItem = true;
    }

    if (!isVFSorFileSystemItem) {
        return false;
    }

    if (!isVFSItem) {
        if (left.parent().data(Qt::UserRole).toString().compare("computer:///") == 0
                && right.parent().data(Qt::UserRole).toString().compare("computer:///") == 0) {
            if (leftItem->uri().compare("file:///") == 0) {
                return false;
            }
            if (rightItem->uri().compare("file:///") == 0) {
                return true;
            }
            if (leftItem->uri().compare("computer:///lingmo-data-volume") == 0) {
                return false;
            }
            if (rightItem->uri().compare("computer:///lingmo-data-volume") == 0) {
                return true;
            }

            if ((!(leftItem->isEjectable() || leftItem->isStopable()) && !leftItem->getDevice().startsWith("/dev/bus/usb"))
                    && (!(rightItem->isEjectable() || rightItem->isStopable()) && !rightItem->getDevice().startsWith("/dev/bus/usb"))) {
                return m_comparer.compare(leftItem->getDevice(), rightItem->getDevice()) > 0;
            } else if (!(leftItem->isEjectable() || leftItem->isStopable()) && !leftItem->getDevice().startsWith("/dev/bus/usb")) {
                return false;
            } else if (!(rightItem->isEjectable() || rightItem->isStopable()) && !rightItem->getDevice().startsWith("/dev/bus/usb")) {
                return true;
            }

            if (((leftItem->isEjectable() || leftItem->isStopable()) && !leftItem->getDevice().startsWith("/dev/sr"))
                    && ((rightItem->isEjectable() || rightItem->isStopable()) && !rightItem->getDevice().startsWith("/dev/sr"))) {
                return m_comparer.compare(leftItem->getDevice(), rightItem->getDevice()) > 0;
            } else if ((leftItem->isEjectable() || leftItem->isStopable()) && !leftItem->getDevice().startsWith("/dev/sr")) {
                return false;
            } else if ((rightItem->isEjectable() || rightItem->isEjectable()) && !rightItem->getDevice().startsWith("/dev/sr")) {
                return true;
            }

            if (leftItem->getDevice().startsWith("/dev/bus/usb") && rightItem->getDevice().startsWith("/dev/bus/usb")) {
                return m_comparer.compare(leftItem->getDevice(), rightItem->getDevice()) > 0;
            } else if (leftItem->getDevice().startsWith("/dev/bus/usb")) {
                return false;
            } else if (rightItem->getDevice().startsWith("/dev/bus/usb")) {
                return true;
            }

            return m_comparer.compare(leftItem->getDevice(), rightItem->getDevice()) > 0;
        }
    }

    return m_comparer.compare(leftItem->displayName(), rightItem->displayName()) > 0;
}

SideBarAbstractItem *SideBarProxyFilterSortModel::itemFromIndex(const QModelIndex &proxy_index)
{
    SideBarModel *model = static_cast<SideBarModel*>(sourceModel());
    auto index = mapToSource(proxy_index);
    return model->itemFromIndex(index);
}
