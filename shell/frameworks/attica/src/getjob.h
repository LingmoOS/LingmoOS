/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_GETJOB_H
#define ATTICA_GETJOB_H

#include <QNetworkRequest>

#include "attica_export.h"
#include "atticabasejob.h"

namespace Attica
{

/**
 * @class GetJob getjob.h <Attica/GetJob>
 *
 * Represents a get job.
 */
class ATTICA_EXPORT GetJob : public Attica::BaseJob
{
    Q_OBJECT

protected:
    GetJob(PlatformDependent *internals, const QNetworkRequest &request);

private:
    QNetworkReply *executeRequest() override;
    const QNetworkRequest m_request;
};

}

#endif
