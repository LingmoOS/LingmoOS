/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QList>
#include <QStringList>
#include <QTime>

#include <KJob>

#include <BluezQt/ObexTransfer>

namespace BluezQt
{
class ObexObjectPush;
}

class SendFilesJob : public KJob
{
    Q_OBJECT

public:
    explicit SendFilesJob(const QStringList &files, BluezQt::DevicePtr device, const QDBusObjectPath &session, QObject *parent = nullptr);

    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void doStart();
    void nextJob();
    void sendFileFinished(BluezQt::PendingCall *call);
    void jobDone();
    void transferredChanged(quint64 transferred);
    void statusChanged(BluezQt::ObexTransfer::Status status);

private:
    void progress(quint64 transferBytes);

    QTime m_time;
    QStringList m_files;
    QList<quint64> m_filesSizes;
    QString m_currentFile;
    quint64 m_progress;
    quint64 m_totalSize;
    qulonglong m_speedBytes;
    quint64 m_currentFileSize;
    quint64 m_currentFileProgress;

    BluezQt::DevicePtr m_device;
    BluezQt::ObexTransferPtr m_transfer;
    BluezQt::ObexObjectPush *m_objectPush;
};
