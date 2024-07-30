/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_SOLIDDEFS_P_H
#define SOLID_SOLIDDEFS_P_H

// clang-format off

#define return_SOLID_CALL(Type, Object, Default, Method) \
    Type t = qobject_cast<Type>(Object); \
    if (t!=nullptr) \
    { \
        return t->Method; \
    } \
    else \
    { \
        return Default; \
    }

#define SOLID_CALL(Type, Object, Method) \
    Type t = qobject_cast<Type>(Object); \
    if (t!=nullptr) \
    { \
        t->Method; \
    }

#endif
