/*
    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QObject>

class KateWaiter : public QObject
{
    Q_OBJECT

public:
    KateWaiter(const QString &service, const QStringList &tokens);

public Q_SLOTS:
    static void exiting();

    void documentClosed(const QString &token);

    static void serviceOwnerChanged(const QString &, const QString &, const QString &);

private:
    QStringList m_tokens;
    QDBusServiceWatcher m_watcher;
};
