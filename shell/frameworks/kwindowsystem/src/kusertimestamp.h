/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Luboš Luňák <l.lunak@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KUSERTIMESTAMP_H
#define KUSERTIMESTAMP_H

#include <kwindowsystem_export.h>

namespace KUserTimestamp
{
/**
 * Returns the last user action timestamp or 0 if no user activity has taken place yet.
 * @see updateuserTimestamp
 */
KWINDOWSYSTEM_EXPORT unsigned long userTimestamp();

/**
 * Updates the last user action timestamp to the given time, or to the current time,
 * if 0 is given. Do not use unless you're really sure what you're doing.
 * Consult focus stealing prevention section in kdebase/kwin/README.
 */
KWINDOWSYSTEM_EXPORT void updateUserTimestamp(unsigned long time = 0);
}

#endif
