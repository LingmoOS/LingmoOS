/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_LEADVERTISINGMANAGER_P_H
#define BLUEZQT_LEADVERTISINGMANAGER_P_H

#include "bluezleadvertisingmanager1.h"

namespace BluezQt
{
typedef org::bluez::LEAdvertisingManager1 BluezLEAdvertisingManager;

class LEAdvertisingManagerPrivate
{
public:
    QString m_path;
    BluezLEAdvertisingManager *m_bluezLEAdvertisingManager = nullptr;
};

} // namespace BluezQt

#endif // BLUEZQT_LEADVERTISINGMANAGER_P_H
