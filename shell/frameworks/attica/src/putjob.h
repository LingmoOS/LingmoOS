/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ATTICA_PUTJOB_H
#define ATTICA_PUTJOB_H

#include <QNetworkRequest>

#include "attica_export.h"
#include "atticabasejob.h"

// workaround to get initialization working with gcc < 4.4
typedef QMap<QString, QString> StringMap;

namespace Attica
{
class Provider;

/**
 * @class PutJob putjob.h <Attica/PutJob>
 *
 * Represents a put job.
 */
class ATTICA_EXPORT PutJob : public BaseJob
{
    Q_OBJECT

protected:
    PutJob(PlatformDependent *internals, const QNetworkRequest &request, QIODevice *data);
    PutJob(PlatformDependent *internals, const QNetworkRequest &request, const StringMap &parameters = StringMap());
    PutJob(PlatformDependent *internals, const QNetworkRequest &request, const QByteArray &byteArray);

private:
    QNetworkReply *executeRequest() override;
    void parse(const QString &) override;

    QIODevice *m_ioDevice;
    QByteArray m_byteArray;

    QString m_responseData;
    const QNetworkRequest m_request;

    QString m_status;
    QString m_statusMessage;

    friend class Attica::Provider;
};

}

#endif
