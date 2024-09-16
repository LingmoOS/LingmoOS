// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DDEVICEDISKINFO_H
#define DDEVICEDISKINFO_H

#include "../dglobal.h"
#include "ddiskinfo.h"

class DDeviceDiskInfoPrivate;
class DDeviceDiskInfo : public DDiskInfo
{
public:
    explicit DDeviceDiskInfo();
    explicit DDeviceDiskInfo(const QString &filePath);

    static QList<DDeviceDiskInfo> localeDiskList();

private:
    DG_DFUNC(DDeviceDiskInfo)
};

#endif // DDEVICEDISKINFO_H
