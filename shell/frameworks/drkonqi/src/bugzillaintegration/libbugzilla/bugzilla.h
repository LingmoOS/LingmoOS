/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUGZILLA_H
#define BUGZILLA_H

#include "connection.h"
#include "models/logindetails.h"

namespace Bugzilla
{
QString version(KJob *kjob);
APIJob *version(const Connection &connection = Bugzilla::connection());

// https://bugzilla.readthedocs.io/en/5.0/api/core/v1/user.html#login
LoginDetails login(KJob *kjob);
APIJob *login(const QString &username, const QString &password, const Connection &connection = Bugzilla::connection());
} // namespace Bugzilla

#endif // BUGZILLA_H
