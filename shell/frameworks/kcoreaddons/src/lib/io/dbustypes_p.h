// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#ifndef DBUSTYPES_P_H
#define DBUSTYPES_P_H

#include <QDBusUnixFileDescriptor>
#include <QList>

using FDList = QList<QDBusUnixFileDescriptor>;

#endif // DBUSTYPES_P_H
