// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kruntimeplatform.h"

QStringList KRuntimePlatform::runtimePlatform()
{
    const QString env = QString::fromLocal8Bit(getenv("PLASMA_PLATFORM"));
    return QStringList(env.split(QLatin1Char(':'), Qt::SkipEmptyParts));
}
