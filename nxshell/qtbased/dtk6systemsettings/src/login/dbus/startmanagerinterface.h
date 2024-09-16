// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dtklogin_global.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>
#include <qobject.h>

using DTK_CORE_NAMESPACE::DDBusInterface;

class QDBusConnection;

DLOGIN_BEGIN_NAMESPACE

class StartManagerInterface : public QObject
{
    Q_OBJECT

public:
    static inline const char *staticInterfaceName() { return "com.deepin.StartManager"; }

    StartManagerInterface(const QString &service,
                          const QString &path,
                          QDBusConnection connection,
                          QObject *parent = nullptr);

Q_SIGNALS:
    void autostartChanged(const QString &status, const QString &name);

public Q_SLOTS:
    QDBusPendingReply<bool> addAutostart(const QString &fileName);
    QDBusPendingReply<bool> removeAutostart(const QString &fileName);

private:
    DDBusInterface *m_interface;
};

DLOGIN_END_NAMESPACE
