// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddisksizeformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

DDiskSizeFormatter::DDiskSizeFormatter()
    : DAbstractUnitFormatter()
{
}

QString DDiskSizeFormatter::unitStr(int unitId) const
{
    switch (unitId) {
        case B:
            return QStringLiteral("B");
        case K:
            return QStringLiteral("KB");
        case M:
            return QStringLiteral("MB");
        case G:
            return QStringLiteral("GB");
        case T:
            return QStringLiteral("TB");
    }

    return QString();
}

DDiskSizeFormatter DDiskSizeFormatter::rate(int rate)
{
    m_rate = rate;

    return *this;
}

DCORE_END_NAMESPACE
