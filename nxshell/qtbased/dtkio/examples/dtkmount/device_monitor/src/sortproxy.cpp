// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sortproxy.h"
#include "devicedef.h"
#include <QDebug>

SortProxy::SortProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

SortProxy::~SortProxy() {}

bool SortProxy::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    QVariant variant = sourceLeft.data(Qt::UserRole);
    DeviceData dataLeft = variant.value<DeviceData>();
    variant = sourceRight.data(Qt::UserRole);
    DeviceData dataRight = variant.value<DeviceData>();

    if (dataLeft.isMount == dataRight.isMount) {
        return dataLeft.deviceName.toLower() <= dataRight.deviceName.toLower();
    } else if (dataLeft.isMount == true && dataRight.isMount == false) {
        return true;
    }
    return false;
}