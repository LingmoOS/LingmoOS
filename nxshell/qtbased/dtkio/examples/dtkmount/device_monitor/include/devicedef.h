// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICEDEF_H
#define DEVICEDEF_H

#include <QMetaType>

struct DeviceData
{
    bool isMount;
    quint64 size;
    quint64 usedSize;
    QString deviceName;
    QString devicePath;
    QString mountPoint;
};

Q_DECLARE_METATYPE(DeviceData)
#endif  // DEVICEDEF_H