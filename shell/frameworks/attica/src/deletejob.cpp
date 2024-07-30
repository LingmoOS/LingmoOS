/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "deletejob.h"

#include <QNetworkAccessManager>

#include "platformdependent_v2.h"

using namespace Attica;

DeleteJob::DeleteJob(PlatformDependent *internals, const QNetworkRequest &request)
    : BaseJob(internals)
    , m_request(request)
{
}

QNetworkReply *DeleteJob::executeRequest()
{
    Attica::PlatformDependentV2 *platformDependentV2 = dynamic_cast<Attica::PlatformDependentV2 *>(internals());
    if (!platformDependentV2) {
        return nullptr;
    }

    return platformDependentV2->deleteResource(m_request);
}

#include "moc_deletejob.cpp"
