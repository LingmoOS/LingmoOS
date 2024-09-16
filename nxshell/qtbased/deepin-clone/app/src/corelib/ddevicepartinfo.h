// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DDEVICEPARTINFO_H
#define DDEVICEPARTINFO_H

#include "../dglobal.h"
#include "dpartinfo.h"

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

class DDevicePartInfoPrivate;
class DDevicePartInfo : public DPartInfo
{
public:
    explicit DDevicePartInfo();
    explicit DDevicePartInfo(const QString &filePath);

    static QList<DDevicePartInfo> localePartList();

private:
    void init(const QJsonObject &obj);

    DG_DFUNC(DDevicePartInfo)

    friend class DDeviceDiskInfoPrivate;
};

#endif // DDEVICEPARTINFO_H
