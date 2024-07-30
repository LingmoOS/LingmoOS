/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHERBACKEND_H
#define KCOLORSCHEMEWATCHERBACKEND_H

#include <QObject>

#include "kcolorschemewatcher.h"

class KColorSchemeWatcherBackend : public QObject
{
    Q_OBJECT
public:
    virtual KColorSchemeWatcher::ColorPreference systemPreference() const = 0;

Q_SIGNALS:
    void systemPreferenceChanged();
};

#endif
