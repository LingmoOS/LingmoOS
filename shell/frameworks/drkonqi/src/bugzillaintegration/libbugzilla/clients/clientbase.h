/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef CLIENTBASE_H
#define CLIENTBASE_H

#include "connection.h"

namespace Bugzilla
{
class ClientBase
{
public:
    explicit ClientBase(const Connection &connection = Bugzilla::connection());

protected:
    const Connection &m_connection;
};

} // namespace Bugzilla

#endif // CLIENTBASE_H
