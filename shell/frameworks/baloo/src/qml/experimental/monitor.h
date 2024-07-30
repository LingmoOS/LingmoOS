/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOOMONITOR_MONITOR_H
#define BALOOMONITOR_MONITOR_H

#include <QDeadlineTimer>
#include <QObject>
#include <QString>
#include <qqmlregistration.h>

#include "indexerstate.h"
#include "schedulerinterface.h"
#include "fileindexerinterface.h"

struct BalooForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(Global)
    QML_FOREIGN_NAMESPACE(Baloo)
};

namespace Baloo {
class Monitor : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filePath READ filePath NOTIFY newFileIndexed)
    Q_PROPERTY(QString suspendState READ suspendState NOTIFY indexerStateChanged)
    Q_PROPERTY(bool balooRunning MEMBER m_balooRunning NOTIFY balooStateChanged)
    Q_PROPERTY(uint totalFiles MEMBER m_totalFiles NOTIFY totalFilesChanged)
    Q_PROPERTY(uint filesIndexed MEMBER m_filesIndexed NOTIFY newFileIndexed)
    Q_PROPERTY(QString remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    Q_PROPERTY(QString stateString READ stateString NOTIFY indexerStateChanged)
    Q_PROPERTY(Baloo::IndexerState state READ state NOTIFY indexerStateChanged)
public:
    explicit Monitor(QObject* parent = nullptr);

    // Property readers
    QString filePath() const { return m_filePath; }
    QString suspendState() const;
    QString remainingTime() const { return m_remainingTime; }
    QString stateString() const { return Baloo::stateString(m_indexerState); }
    Baloo::IndexerState state() const { return m_indexerState; }

    // Invokable methods
    Q_INVOKABLE void toggleSuspendState();
    Q_INVOKABLE void startBaloo();

Q_SIGNALS:
    void newFileIndexed();
    void balooStateChanged();
    void totalFilesChanged();
    void remainingTimeChanged();
    void indexerStateChanged();

private Q_SLOTS:
    void newFile(const QString& filePath);
    void balooStarted();
    void slotIndexerStateChanged(int state);

private:
    void fetchTotalFiles();
    void updateRemainingTime();

    QDBusConnection m_bus;

    QString m_filePath;
    bool m_balooRunning = false;
    Baloo::IndexerState m_indexerState = Baloo::Unavailable;
    QDeadlineTimer m_remainingTimeTimer = QDeadlineTimer(0);

    org::kde::baloo::scheduler* m_scheduler;
    org::kde::baloo::fileindexer* m_fileindexer;

    uint m_totalFiles = 0;
    uint m_filesIndexed = 0;
    QString m_remainingTime;
    uint m_remainingTimeSeconds = 0;
};
}
#endif //BALOOMONITOR_MONITOR_H
