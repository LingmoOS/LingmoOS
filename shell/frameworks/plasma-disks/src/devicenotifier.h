// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

class Device;

/** Abstraction interface for device discovery. Implemented as a Solid variant for example. */
class DeviceNotifier : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual void start() = 0;
    virtual void loadData() = 0;

Q_SIGNALS:
    void addDevice(Device *device);
    void removeUDI(const QString &udi);
};
