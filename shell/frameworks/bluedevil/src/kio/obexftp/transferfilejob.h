/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QTime>

#include <KJob>

#include <BluezQt/ObexTransfer>

class KioFtp;

class TransferFileJob : public KJob
{
    Q_OBJECT

public:
    explicit TransferFileJob(BluezQt::ObexTransferPtr transfer, KioFtp *parent = nullptr);

    void start() override;

private Q_SLOTS:
    void doStart();
    void statusChanged(BluezQt::ObexTransfer::Status status);
    void transferredChanged(quint64 transferred);

private:
    QTime m_time;
    qlonglong m_speedBytes;
    KioFtp *const m_parent;
    BluezQt::ObexTransferPtr m_transfer;
};
