/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2011 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_SOLIDNAMESPACE_H
#define SOLID_SOLIDNAMESPACE_H

namespace Solid
{
enum ErrorType {
    NoError = 0,
    UnauthorizedOperation,
    DeviceBusy,
    OperationFailed,
    UserCanceled,
    InvalidOption,
    MissingDriver,
};
}

#include <QMetaType>

Q_DECLARE_METATYPE(Solid::ErrorType)

#endif
