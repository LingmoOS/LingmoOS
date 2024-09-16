// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "signaldemo.h"

#include "dloginsession.h"

#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>

SignalDemo::SignalDemo(QObject *parent)
    : LoginDemo(parent)
    , m_manager(new DLoginManager)
{
    auto current = m_manager->currentSession();
    if (current) {
        m_currentSession = current.value();
    } else {
        m_currentSession = {};
    }
}

int SignalDemo::run()
{
    connect(m_currentSession.data(), &DLoginSession::lockedChanged, this, [&](bool locked) {
        qDebug() << "Lock changed:" << locked;
    });
    connect(m_manager, &DLoginManager::userNew, this, [=](quint32 UID) {
        qDebug() << "User" << UID << "is added.";
    });
    connect(m_manager, &DLoginManager::userRemoved, this, [=](quint32 UID) {
        qDebug() << "User" << UID << "is removed.";
    });
    connect(m_manager, &DLoginManager::sessionNew, this, [=](const QString sessionId) {
        qDebug() << "Session" << sessionId << "is added.";
    });
    connect(m_manager, &DLoginManager::sessionRemoved, this, [=](const QString &sessionId) {
        qDebug() << "Session" << sessionId << "is removed.";
    });
    connect(m_currentSession.data(), &DLoginSession::autostartAdded, this, [=](const QString name) {
        qDebug() << "Autostart" << name << "is added.";
    });
    connect(m_currentSession.data(),
            &DLoginSession::autostartRemoved,
            this,
            [=](const QString &name) {
                qDebug() << "Autostart" << name << "is removed.";
            });
    connect(m_manager, &DLoginManager::prepareForSleep, this, [=](bool value) {
        qDebug() << "Prepare for sleep, value:" << value;
    });
    connect(m_manager, &DLoginManager::prepareForShutdown, this, [=](bool value) {
        qDebug() << "Prepare for shutdown, value:" << value;
        QFile file(QDir::currentPath() + "/shutdown.log");
        while (!file.open(QIODevice::ReadWrite | QIODevice::Append))
            ;
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString() << ": value = " << value;
        file.close();
    });

    return 0;
}
