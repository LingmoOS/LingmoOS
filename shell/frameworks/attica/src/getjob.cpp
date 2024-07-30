/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "getjob.h"

#include <QNetworkAccessManager>

#include "platformdependent.h"
#include <attica_debug.h>

using namespace Attica;

GetJob::GetJob(PlatformDependent *internals, const QNetworkRequest &request)
    : BaseJob(internals)
    , m_request(request)
{
}

QNetworkReply *GetJob::executeRequest()
{
    return internals()->get(m_request);
}

#include "moc_getjob.cpp"
