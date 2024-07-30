/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QTime>
#include <QUrl>

#include <KJob>

#include <BluezQt/ObexTransfer>
#include <BluezQt/Request>

class ObexAgent;

class ReceiveFileJob : public KJob
{
    Q_OBJECT

public:
    explicit ReceiveFileJob(const BluezQt::Request<QString> &req, BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, ObexAgent *parent);

    QString deviceAddress() const;

    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void init();
    void showNotification();
    void slotCancel();
    void slotAccept();
    void moveFinished(KJob *job);

    void statusChanged(BluezQt::ObexTransfer::Status status);
    void transferredChanged(quint64 transferred);

private:
    QString createTempPath(const QString &fileName) const;

    QTime m_time;
    qulonglong m_speedBytes;
    QString m_tempPath;
    QString m_deviceName;
    QString m_deviceAddress;
    QUrl m_targetPath;

    ObexAgent *m_agent;
    BluezQt::ObexTransferPtr m_transfer;
    BluezQt::ObexSessionPtr m_session;
    BluezQt::Request<QString> m_request;
    bool m_accepted = false;
};
