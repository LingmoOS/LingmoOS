/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICAUTILS_H
#define ATTICAUTILS_H

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QString>

namespace Attica
{
class Utils
{
public:
    /// parses the QtDateTime in ISO 8601 format correctly (recognizes TZ properly)
    static QDateTime parseQtDateTimeIso8601(const QString &str);
    static const char *toString(QNetworkAccessManager::Operation operation);
};

}

#endif // ATTICAUTILS_H
