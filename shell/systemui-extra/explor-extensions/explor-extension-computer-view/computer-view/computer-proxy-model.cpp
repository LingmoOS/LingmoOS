/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "computer-proxy-model.h"
#include "computer-model.h"
#include "abstract-computer-item.h"

#include <QDebug>

static ComputerProxyModel *global_instance = nullptr;

ComputerProxyModel::ComputerProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    auto computerModel = new ComputerModel(this);
    m_model = computerModel;
    setSourceModel(computerModel);

    m_locale = QLocale(QLocale::system().name());
    m_collator = QCollator(m_locale);
    m_collator.setNumericMode(true);

    connect(m_model, &ComputerModel::updateLocationRequest, this, &ComputerProxyModel::updateLocationRequest);
    connect(m_model, &ComputerModel::invalidateRequest, this, &ComputerProxyModel::invalidateFilter);
    connect(m_model, &ComputerModel::updateRequest, this, &ComputerProxyModel::updateRequest);
}

ComputerProxyModel *ComputerProxyModel::globalInstance()
{
    if (!global_instance) {
        global_instance = new ComputerProxyModel;
    }
    return global_instance;
}

AbstractComputerItem *ComputerProxyModel::itemFromIndex(const QModelIndex &proxyIndex)
{
    auto index = mapToSource(proxyIndex);
    return static_cast<AbstractComputerItem *>(index.internalPointer());
}

QString ComputerProxyModel::tryGetVolumeUriFromMountTarget(const QString &mountTargetUri)
{
    return m_model->tryGetVolumeUriFromMountRoot(mountTargetUri);
}

QString ComputerProxyModel::tryGetVolumeRealUriFromUri(const QString &uri)
{
    return m_model->tryGetVolumeRealUriFromUri(uri);
}

void ComputerProxyModel::refresh()
{
    m_model->refresh();
}

bool ComputerProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto item = static_cast<AbstractComputerItem *>(m_model->index(source_row, 0, source_parent).internalPointer());
    return !item->isHidden();
}

bool ComputerProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (!source_left.parent().isValid() && source_right.parent().isValid()) {
        return sortOrder() == Qt::AscendingOrder ? false : true;
    }

    if (!(source_left.isValid() && source_right.isValid())) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }

    auto leftItem = static_cast<AbstractComputerItem*>(source_left.internalPointer());
    auto rightItem = static_cast<AbstractComputerItem*>(source_right.internalPointer());
    if (leftItem->itemType() != AbstractComputerItem::Volume || rightItem->itemType() != AbstractComputerItem::Volume) {
        return false;
    }

    if (leftItem->uri().compare("file:///") == 0) {
        return true;
    }
    if (rightItem->uri().compare("file:///") == 0) {
        return false;
    }

    if (leftItem->uri().compare("computer:///lingmo-data-volume") == 0) {
        return true;
    }
    if (rightItem->uri().compare("computer:///lingmo-data-volume") == 0) {
        return false;
    }

    if (!leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/bus/usb")
            && !rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
    } else if (!leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return true;
    } else if (!rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return false;
    }

    if ((leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/sr"))
            && (rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/sr"))) {
        return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
    } else if (leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/sr")) {
        return true;
    } else if (rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/sr")) {
        return false;
    }

    if (leftItem->m_unixDeviceName.startsWith("/dev/bus/usb") && rightItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
    } else if (leftItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return true;
    } else if (rightItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return false;
    }
    return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
}
