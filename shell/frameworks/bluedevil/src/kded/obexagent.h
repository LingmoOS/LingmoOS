/*************************************************************************************
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>               *
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>                           *
 *                                                                                   *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *************************************************************************************/

#pragma once

#include <BluezQt/ObexAgent>
#include <QHash>
class KJob;

class BlueDevilDaemon;

class ObexAgent : public BluezQt::ObexAgent
{
    Q_OBJECT

public:
    explicit ObexAgent(BlueDevilDaemon *daemon);

    BluezQt::Manager *manager() const;

    bool shouldAutoAcceptTransfer(const QString &address) const;

    QDBusObjectPath objectPath() const override;
    void authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request) override;

private Q_SLOTS:
    void receiveFileJobFinished(KJob *job);

private:
    BluezQt::Manager *const m_manager;
    QHash<QString, QDateTime> m_transferTimes;
};
