// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef PARTITIONLISTITEM_H
#define PARTITIONLISTITEM_H

#include "utilitylistdeviceitem.h"
#include "../corelib/dpartinfo.h"

class PartitionListItem : public UtilityListDeviceItem
{
    Q_OBJECT

public:
    explicit PartitionListItem(DPartInfo info, QWidget *parent = 0);

    DPartInfo info() const;

private:
    DPartInfo m_info;
};

#endif // PARTITIONLISTITEM_H
