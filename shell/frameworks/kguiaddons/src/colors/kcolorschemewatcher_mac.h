/*
 * SPDX-FileCopyrightText: 2022 Jyrki Gadinger <nilsding@nilsding.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHER_MAC_H
#define KCOLORSCHEMEWATCHER_MAC_H

#include "kcolorschemewatcherbackend.h"

class KColorSchemeWatcherMac : public KColorSchemeWatcherBackend
{
    Q_OBJECT

public:
    explicit KColorSchemeWatcherMac();
    ~KColorSchemeWatcherMac();

    KColorSchemeWatcher::ColorPreference systemPreference() const override;

private:
    // not ideal, in obj-c++ this would be an `id`, but since this header is
    // included from pure C++ files the compiler will not know what to do with
    // an `id`
    void *m_observer;
};

#endif
