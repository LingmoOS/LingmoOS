/*
 * SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
 * SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DEVICERECEIVERTEST_H
#define DEVICERECEIVERTEST_H

#include <QObject>

#include "manager.h"

class DeviceReceiver : public QObject
{
    Q_OBJECT

public:
    explicit DeviceReceiver(QObject *parent = nullptr);

    void scanDevices();

public Q_SLOTS:
    void deviceFound(const BluezQt::DevicePtr &device);
    void devicePropertyChanged(const BluezQt::DevicePtr &device);
    void adapterAdded(const BluezQt::AdapterPtr &adapter);
};

#endif // DEVICERECEIVERTEST_H
