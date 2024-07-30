/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_CALENDARPLUGINLOADER_H
#define KCALENDARCORE_CALENDARPLUGINLOADER_H

#include "kcalendarcore_export.h"

#include <KCalendarCore/CalendarPlugin>

namespace KCalendarCore
{

/**
 * Provides access to a KCalendarCore::CalendarPlugin instance, if available.
 * @since 5.97
 */
class KCALENDARCORE_EXPORT CalendarPluginLoader
{
    Q_GADGET
    Q_PROPERTY(bool hasPlugin READ hasPlugin)
    Q_PROPERTY(KCalendarCore::CalendarPlugin *plugin READ plugin)

public:
    /** Returns @c true if there is a platform calendar available. */
    static bool hasPlugin();

    /** Returns the platform calendar plugin. */
    static KCalendarCore::CalendarPlugin *plugin();
};

}

#endif // KCALENDARCORE_CALENDARPLUGINLOADER_H
