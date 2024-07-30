/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIA_P_H
#define BLUEZQT_MEDIA_P_H

#include "bluezmedia1.h"

namespace BluezQt
{
typedef org::bluez::Media1 BluezMedia;

class MediaPrivate
{
public:
    QString m_path;
    BluezMedia *m_bluezMedia = nullptr;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIA_P_H
