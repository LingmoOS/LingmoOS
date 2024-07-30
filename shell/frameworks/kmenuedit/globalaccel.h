/*
 * SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef GLOBALACCEL_H
#define GLOBALACCEL_H

#include <KService>
#include <QKeySequence>

class GlobalAccel
{
public:
    static QKeySequence getMenuEntryShortcut(const KService::Ptr &service);
    static void changeMenuEntryShortcut(const KService::Ptr &service, const QKeySequence &shortcut);
};

#endif // GLOBALACCEL_H
