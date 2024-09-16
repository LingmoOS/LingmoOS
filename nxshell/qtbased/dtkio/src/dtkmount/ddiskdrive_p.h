// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDISKDRIVE_P_H
#define DDISKDRIVE_P_H

#include <DDiskDrive>

#include <QDBusError>

class OrgFreedesktopUDisks2DriveInterface;

DMOUNT_BEGIN_NAMESPACE

class DDiskDrivePrivate
{
public:
    OrgFreedesktopUDisks2DriveInterface *dbus { nullptr };
    QDBusError err;
};

DMOUNT_END_NAMESPACE

#endif   // DDISKDRIVE_P_H
