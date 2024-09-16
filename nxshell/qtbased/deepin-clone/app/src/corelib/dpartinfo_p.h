// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DPARTINFO_P_H
#define DPARTINFO_P_H

#include <QSharedData>

#include "dpartinfo.h"

class DPartInfoPrivate : public QSharedData
{
public:
    DPartInfoPrivate(DPartInfo *qq);

    virtual void refresh();
    virtual qint64 getUsedSize() const;
    virtual qint64 getFreeSize() const;
    virtual bool getIsDeepinSystemRoot() const;

    DPartInfo::FSType toType(const QString &name);

    DPartInfo *q;

    QString parentDiskFilePath;
    QString filePath;
    QString name;
    QString kname;
    int index = 0;
    int blockSize = 4096;
    // Unit: bytes
    qint64 sizeStart = 0;
    qint64 sizeEnd = 0;
    qint64 size = 0;
    qint64 usedSize = 0;
    qint64 freeSize = 0;
    QString fsTypeName;
    DPartInfo::FSType fsType;
    QString mountPoint;
    QString uuid;
    QString label;
    QString partLabel;
    DPartInfo::Type partType;
    QString partTypeName;
    DPartInfo::GUIDType guidType;

    bool readonly = false;
    bool removeable = false;
    bool isDeepinSystemRoot = false;
    QString transport;
    QString partUUID;
};

#endif // DPARTINFO_P_H
