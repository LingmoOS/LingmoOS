/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHER_XDG_H
#define KCOLORSCHEMEWATCHER_XDG_H

#include "kcolorschemewatcherbackend.h"

#include <QDBusVariant>

class KColorSchemeWatcherXDG : public KColorSchemeWatcherBackend
{
    Q_OBJECT
public:
    KColorSchemeWatcherXDG();
    KColorSchemeWatcher::ColorPreference systemPreference() const override;

private Q_SLOTS:
    void slotSettingChanged(QString, QString, QDBusVariant);

private:
    KColorSchemeWatcher::ColorPreference fdoToInternal(uint value) const;
    KColorSchemeWatcher::ColorPreference m_preference = KColorSchemeWatcher::NoPreference;
};

#endif
