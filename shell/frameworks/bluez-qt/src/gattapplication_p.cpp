/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattapplication_p.h"

namespace BluezQt
{
GattApplicationPrivate::GattApplicationPrivate(const QString &objectPathPrefix, GattApplication *q_ptr)
    : q(q_ptr)
{
    static uint8_t appNumber = 0;
    m_objectPath.setPath(objectPathPrefix + QStringLiteral("/app") + QString::number(appNumber++));
}

} // namespace BluezQt
