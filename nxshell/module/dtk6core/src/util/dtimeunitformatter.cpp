// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtimeunitformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

DTimeUnitFormatter::DTimeUnitFormatter()
    : DAbstractUnitFormatter()
{
}

uint DTimeUnitFormatter::unitConvertRate(int unitId) const
{
    switch (unitId) {
        case Seconds:
            return 60;
        case Minute:
            return 60;
        case Hour:
            return 24;
        default:;
    }

    return 0;
}

QString DTimeUnitFormatter::unitStr(int unitId) const
{
    switch (unitId) {
        case Seconds:
            return QStringLiteral("s");
        case Minute:
            return QStringLiteral("m");
        case Hour:
            return QStringLiteral("h");
        case Day:
            return QStringLiteral("d");
        default:;
    }

    return QString();
}

DCORE_END_NAMESPACE
