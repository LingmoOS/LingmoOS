/*
 * SPDX-FileCopyrightText: 2011 Rafael Fernández López <ereslibre@kde.org>
 * SPDX-FileCopyrightText: 2011 UFO Coders <info@ufocoders.com>
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ADAPTERSRECEIVER_H
#define ADAPTERSRECEIVER_H

#include <QObject>

#include "manager.h"

class AdaptersReceiver : public QObject
{
    Q_OBJECT

public:
    explicit AdaptersReceiver(BluezQt::Manager *manager, QObject *parent = nullptr);

public Q_SLOTS:
    void adapterAdded(const BluezQt::AdapterPtr &adapter);
    void adapterRemoved(const BluezQt::AdapterPtr &adapter);
    void usableAdapterChanged(const BluezQt::AdapterPtr &adapter);
    void allAdaptersRemoved();
    void bluetoothOperationalChanged(bool operational);
    void bluetoothBlockedChanged(bool blocked);
    void printStatus();

private:
    BluezQt::Manager *m_manager;
};

#endif // ADAPTERSRECEIVER_H
