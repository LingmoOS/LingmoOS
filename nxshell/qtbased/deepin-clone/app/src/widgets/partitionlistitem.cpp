// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "partitionlistitem.h"

#include <QIcon>
#include <QDebug>

PartitionListItem::PartitionListItem(DPartInfo info, QWidget *parent)
    : UtilityListDeviceItem(parent)
    , m_info(info)
{
    setTitle(info.mountPoint().isEmpty() ? info.partLabel() : info.mountPoint());
    setMessage(info.name());
    setSizeInfo(info.usedSize(), info.totalSize());

    QString icon_name;

    if (info.isRemoveable()) {
        if (info.transport().isEmpty())
            icon_name = "drive-removable-media";
        else
            icon_name = "drive-removable-media-" + info.transport();
    } else {
        if (info.transport().isEmpty())
            icon_name = "drive-harddisk";
        else
            icon_name = "drive-harddisk-" + info.transport();
    }

    const QIcon icon = QIcon::fromTheme(icon_name);

    setIcon(info.isDeepinSystemRoot() ? QIcon::fromTheme("drive-harddisk-deepin", icon) : icon, QSize(48, 48));
}

DPartInfo PartitionListItem::info() const
{
    return m_info;
}
