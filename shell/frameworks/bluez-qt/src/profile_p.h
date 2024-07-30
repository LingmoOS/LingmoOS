/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_PROFILE_P_H
#define BLUEZQT_PROFILE_P_H

#include <QVariantMap>

namespace BluezQt
{
class ProfilePrivate
{
public:
    QVariantMap options;
};

} // namespace BluezQt

#endif // BLUEZQT_PROFILE_P_H
