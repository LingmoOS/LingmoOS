/*
 *   This file is part of the KDE project
 *
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "kdedobexftp.h"

#include <QObject>

#include <KIO/WorkerBase>

#include <BluezQt/ObexFileTransfer>

class KioFtp : public QObject, public KIO::WorkerBase
{
    Q_OBJECT

public:
    KioFtp(const QByteArray &pool, const QByteArray &app);

    KIO::WorkerResult copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) override;
    KIO::WorkerResult listDir(const QUrl &url) override;
    void setHost(const QString &host, quint16 port, const QString &user, const QString &pass) override;
    KIO::WorkerResult stat(const QUrl &url) override;
    KIO::WorkerResult del(const QUrl &url, bool isfile) override;
    KIO::WorkerResult mkdir(const QUrl &url, int permissions) override;
    KIO::WorkerResult rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags) override;
    KIO::WorkerResult get(const QUrl &url) override;

    bool cancelTransfer(const QString &transfer);

private:
    [[nodiscard]] KIO::WorkerResult copyHelper(const QUrl &src, const QUrl &dest);
    [[nodiscard]] KIO::WorkerResult copyWithinObexftp(const QUrl &src, const QUrl &dest);
    [[nodiscard]] KIO::WorkerResult copyFromObexftp(const QUrl &src, const QUrl &dest);
    [[nodiscard]] KIO::WorkerResult copyToObexftp(const QUrl &src, const QUrl &dest);
    [[nodiscard]] KIO::WorkerResult statHelper(const QUrl &url);

    struct ListResult {
        KIO::WorkerResult result;
        QList<KIO::UDSEntry> entries;
    };
    [[nodiscard]] ListResult listFolder(const QUrl &url);
    [[nodiscard]] KIO::WorkerResult changeFolder(const QString &folder);
    [[nodiscard]] KIO::WorkerResult createFolder(const QString &folder);
    [[nodiscard]] KIO::WorkerResult deleteFile(const QString &file);

    void updateRootEntryIcon(KIO::UDSEntry &entry, const QString &memoryType);
    bool createSession(const QString &target);
    void connectToHost();
    [[nodiscard]] KIO::WorkerResult testConnection();

private:
    QMap<QString, KIO::UDSEntry> m_statMap;
    QString m_host;
    QString m_sessionPath;
    org::kde::BlueDevil::ObexFtp *m_kded;
    BluezQt::ObexFileTransfer *m_transfer;
};
