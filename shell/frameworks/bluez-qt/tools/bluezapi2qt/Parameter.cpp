/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Parameter.h"
#include <QStringList>

Parameter Parameter::fromString(const QString &string)
{
    Parameter param;
    QStringList arg = string.split(QLatin1Char(' '));
    if (arg.size() != 2) {
        return param;
    }

    param.m_type = arg.first();
    param.m_name = arg.last();

    return param;
}

QString Parameter::type() const
{
    return m_type;
}

QString Parameter::name() const
{
    return m_name;
}
