/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "clientbase.h"

namespace Bugzilla
{
ClientBase::ClientBase(const Connection &connection)
    : m_connection(connection)
{
}

} // namespace Bugzilla
