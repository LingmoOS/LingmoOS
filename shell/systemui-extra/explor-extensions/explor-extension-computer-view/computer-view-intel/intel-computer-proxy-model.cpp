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

#include "intel-computer-proxy-model.h"
#include "intel-computer-model.h"
#include "intel-abstract-computer-item.h"

using namespace Intel;

static ComputerProxyModel *global_instance = nullptr;

ComputerProxyModel::ComputerProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    auto computerModel = new ComputerModel(this);
    setSourceModel(computerModel);
    m_model = computerModel;
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

void ComputerProxyModel::refresh()
{
    m_model->refresh();
}

bool ComputerProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto item = static_cast<AbstractComputerItem *>(m_model->index(source_row, 0, source_parent).internalPointer());
    if (item->itemType() == AbstractComputerItem::RemoteVolume) {
        return !item->isHidden();
    }
    return true;
}
