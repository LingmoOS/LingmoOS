/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "logindinhibitionargument.h"

#include <QStringList>

using namespace Solid;

QString LogindInhibitionArgument::fromPowerState(Power::InhibitionTypes states)
{
    QStringList args;
    if (states | Power::Sleep) {
        args << QStringLiteral("sleep");
    }
    if (states | Power::Shutdown) {
        args << QStringLiteral("shutdown");
    }
    return args.join(QChar(':'));
}
