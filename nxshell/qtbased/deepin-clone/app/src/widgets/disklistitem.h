// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISKLISTITEM_H
#define DISKLISTITEM_H

#include "utilitylistdeviceitem.h"
#include "../corelib/ddiskinfo.h"

class DiskListItem : public UtilityListDeviceItem
{
    Q_OBJECT
public:
    explicit DiskListItem(DDiskInfo info, QWidget *parent = 0);

    DDiskInfo info() const;

private:
    DDiskInfo m_info;
};

#endif // DISKLISTITEM_H
