/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ATTICA_POSTJOB_H
#define ATTICA_POSTJOB_H

#include <QNetworkRequest>

#include "attica_export.h"
#include "atticabasejob.h"

// workaround to get initialization working with gcc < 4.4
typedef QMap<QString, QString> StringMap;

namespace Attica
{
class Provider;

/**
 * @class PostJob postjob.h <Attica/PostJob>
 *
 * Represents a post job.
 */
class ATTICA_EXPORT PostJob : public BaseJob
{
    Q_OBJECT

protected:
    PostJob(PlatformDependent *internals, const QNetworkRequest &request, QIODevice *data);
    PostJob(PlatformDependent *internals, const QNetworkRequest &request, const StringMap &parameters = StringMap());
    PostJob(PlatformDependent *internals, const QNetworkRequest &request, const QByteArray &byteArray);

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
