// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defsecuritytoolswineinstaller.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>

DefSecurityToolsWineInstaller::DefSecurityToolsWineInstaller(QObject *parent)
    : DefSecurityToolsBaseInstaller(parent)
{
}

DefSecurityToolsWineInstaller::~DefSecurityToolsWineInstaller()
{
}

void DefSecurityToolsWineInstaller::showtool(const QStringList &strParams)
{
    Q_UNUSED(strParams);
    QDBusInterface inter("com.deepin.SessionManager", "/com/deepin/StartManager", "com.deepin.StartManager");
    inter.call(QDBus::NoBlock, "Launch", m_info.strReserve);
}
