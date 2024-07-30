/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LOGINDETAILS_H
#define LOGINDETAILS_H

#include <QString>

namespace Bugzilla
{
struct LoginDetails {
    int id;
    QString token;
};

} // namespace Bugzilla

#endif // LOGINDETAILS_H
