/*
 * SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHER_QT_H
#define KCOLORSCHEMEWATCHER_QT_H

#include "kcolorschemewatcherbackend.h"

class KColorSchemeWatcherQt : public KColorSchemeWatcherBackend
{
    Q_OBJECT
public:
    KColorSchemeWatcherQt();
    KColorSchemeWatcher::ColorPreference systemPreference() const override;
};

#endif
