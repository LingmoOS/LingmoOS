/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <solid/solidnamespace.h>

static int registerSolidMetaTypes()
{
    qRegisterMetaType<Solid::ErrorType>();

    return 0; // something
}

#ifdef Q_CONSTRUCTOR_FUNCTION
Q_CONSTRUCTOR_FUNCTION(registerSolidMetaTypes)
#else
static const int _Solid_registerMetaTypes = registerSolidMetaTypes();
#endif
