/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

// Self
#include "KSMServer.h"

// Qt
#include <QPair>

// STD
#include <memory>

class QDBusServiceWatcher;
class QDBusInterface;
class QDBusPendingCallWatcher;

class KSMServer::Private : public QObject
{
    Q_OBJECT

public:
    Private(KSMServer *parent);

    void processLater(const QString &activity, bool start);

private Q_SLOTS:
    void process();
    void makeRunning(bool value);

    void startCallFinished(QDBusPendingCallWatcher *watcher);
    void stopCallFinished(QDBusPendingCallWatcher *watcher);

    void subSessionSendEvent(int event);

private:
    QDBusInterface *kwin;

    bool processing;
    QString processingActivity;
    QList<QPair<QString, bool>> queue;

    KSMServer *const q;
};
