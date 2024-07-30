/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZAPI2QT_PARAMETER_H
#define BLUEZAPI2QT_PARAMETER_H

#include <QString>

class Parameter
{
public:
    static Parameter fromString(const QString &string);

    QString type() const;
    QString name() const;

private:
    QString m_type;
    QString m_name;
};

#endif
