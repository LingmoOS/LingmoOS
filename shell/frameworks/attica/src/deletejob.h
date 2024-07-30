/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_DELETEJOB_H
#define ATTICA_DELETEJOB_H

#include <QNetworkRequest>

#include "attica_export.h"
#include "atticabasejob.h"

namespace Attica
{

/**
 * @class DeleteJob deletejob.h <Attica/DeleteJob>
 *
 * Represents a delete job.
 */
class ATTICA_EXPORT DeleteJob : public Attica::BaseJob
{
    Q_OBJECT

protected:
    DeleteJob(PlatformDependent *internals, const QNetworkRequest &request);

private:
    QNetworkReply *executeRequest() override;
    const QNetworkRequest m_request;
};

}

#endif
