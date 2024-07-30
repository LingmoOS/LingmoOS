/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dummyrequeststatejob.h"

#include <Solid/RequestStateJob>

using namespace Solid;

void DummyRequestStateJob::doStart()
{
    if (state == Power::Shutdown) {
        setError(RequestStateJob::Unsupported);
        setErrorText(QLatin1String("State Brightness is unsupported"));
    }
    emitResult();
}

#include "moc_dummyrequeststatejob.cpp"
