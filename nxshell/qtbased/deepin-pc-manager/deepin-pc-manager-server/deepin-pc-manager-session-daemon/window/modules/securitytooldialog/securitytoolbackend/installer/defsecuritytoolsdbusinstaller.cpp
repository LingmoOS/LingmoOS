// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defsecuritytoolsdbusinstaller.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>

DefSecurityToolsDbusInstaller::DefSecurityToolsDbusInstaller(QObject *parent)
    : DefSecurityToolsBaseInstaller(parent)
{
}

DefSecurityToolsDbusInstaller::~DefSecurityToolsDbusInstaller()
{
}

void DefSecurityToolsDbusInstaller::showtool(const QStringList &strParams)
{
    if (m_info.strReserve4.trimmed() == "system") {
        QDBusInterface interface(m_info.strReserve, m_info.strReserve2, m_info.strReserve3, QDBusConnection::systemBus());

        if (!strParams.isEmpty()) {
            QList<QVariant> varlist;

            foreach (QString strParam, strParams) {
                varlist.push_back(strParam);
            }

            interface.callWithArgumentList(QDBus::NoBlock, m_info.strCallMethodName, varlist);
        } else {
            interface.call(m_info.strCallMethodName);
        }
    } else {
        QDBusInterface interface(m_info.strReserve, m_info.strReserve2, m_info.strReserve3, QDBusConnection::sessionBus());

        if (!strParams.isEmpty()) {
            QList<QVariant> varlist;

            foreach (QString strParam, strParams) {
                varlist.push_back(strParam);
            }

            interface.callWithArgumentList(QDBus::NoBlock, m_info.strCallMethodName, varlist);
        } else {
            interface.call(m_info.strCallMethodName);
        }
    }
}
