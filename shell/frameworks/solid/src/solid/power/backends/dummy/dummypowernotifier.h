/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DUMMY_POWER_NOTIFIER_H
#define SOLID_DUMMY_POWER_NOTIFIER_H

#include "backends/powernotifier.h"

namespace Solid
{
class DummyPowerNotifier : public PowerNotifier
{
    Q_OBJECT
public:
    explicit DummyPowerNotifier(QObject *parent = nullptr);

private Q_SLOTS:
    void emitEverythingChanged();
};
}

#endif // SOLID_DUMMY_POWER_NOTIFIER_H
