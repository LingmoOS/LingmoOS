/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTMANAGER_P_H
#define BLUEZQT_GATTMANAGER_P_H

#include "bluezgattmanager1.h"

class QString;

namespace BluezQt
{
class GattManagerPrivate
{
public:
    GattManagerPrivate(const QString &path);
    ~GattManagerPrivate();

    OrgBluezGattManager1Interface m_dbusInterface;
};

} // namespace BluezQt

#endif
