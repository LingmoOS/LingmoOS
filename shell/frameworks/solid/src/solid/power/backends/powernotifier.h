/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_POWER_NOTIFIER_H
#define SOLID_POWER_NOTIFIER_H

#include <QObject>

namespace Solid
{
class PowerNotifier : public QObject
{
    Q_OBJECT
public:
    explicit PowerNotifier(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

Q_SIGNALS:
    void acPluggedChanged(bool plugged);
    void aboutToSuspend();
    void resumeFromSuspend();
};
}

#endif // SOLID_POWER_NOTIFIER_H
