/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KSCREEN_KCM_UTILS_H
#define KSCREEN_KCM_UTILS_H

#include <QSize>
#include <QString>

#include <kscreen/output.h>
#include <kscreen/types.h>

namespace Utils
{
QString outputName(const KScreen::Output *output, bool shouldShowSerialNumber = false, bool shouldShowConnector = false);
QString outputName(const KScreen::OutputPtr &output, bool shouldShowSerialNumber = false, bool shouldShowConnector = false);

QString sizeToString(const QSize &size);
}

#endif
