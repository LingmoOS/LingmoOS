// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusscreenshot.h"

/*
 * Implementation of interface class DBusScreenshot
 */

DBusScreenshot::DBusScreenshot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DBusScreenshot::~DBusScreenshot()
{
}

