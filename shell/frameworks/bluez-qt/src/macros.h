/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MACROS_H
#define BLUEZQT_MACROS_H

// clang-format off

// Change property value and emit propertyChanged signal
#define PROPERTY_CHANGED(var, type_cast, signal) \
    if (var != value.type_cast()) { \
        var = value.type_cast(); \
        Q_EMIT q.lock()->signal(var); \
    }

#define PROPERTY_CHANGED2(var, value, signal) \
    const auto &val = value; \
    if (var != val) { \
        var = val; \
        Q_EMIT q.lock()->signal(var); \
    }

// Clears property value and emit propertyChanged signal
#define PROPERTY_INVALIDATED(var, empty, signal) \
    if (var != empty) { \
        var = empty; \
        Q_EMIT q.lock()->signal(var); \
    }

#endif // BLUEZQT_MACROS_H
