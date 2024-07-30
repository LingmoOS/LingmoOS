/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef TYPEANNOTATION_H
#define TYPEANNOTATION_H

#include <QString>

enum class AnnotationType {
    Bluez = 0,
    Dbus = 1,
    Qt = 2,
};
QString annotateType(AnnotationType from, AnnotationType to, const QString &type);

QString bluezToQt(const QString &type);

#endif // TYPEANNOTATION_H
