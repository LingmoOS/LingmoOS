/*
 *   SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *   SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>

#include <KSharedConfig>

#include <BluezQt/Types>

class KFilePlacesModel;

class BlueDevilDaemon;

class DeviceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit DeviceMonitor(BlueDevilDaemon *daemon);

    void saveState();

private Q_SLOTS:
    void readyToSetInitialState(bool operational);

    void bluetoothOperationalChanged(bool operational);
    void adapterAdded(BluezQt::AdapterPtr adapter);
    void deviceAdded(BluezQt::DevicePtr device);

    void deviceConnectedChanged(bool connected);
    void login1PrepareForSleep(bool active);

private:
    void setInitialState();

    void restoreState();
    void restoreAdapter(BluezQt::AdapterPtr adapter);

    void clearPlaces();
    void updateDevicePlace(BluezQt::DevicePtr device);

    KFilePlacesModel *places();

    BluezQt::Manager *m_manager;
    bool m_isParentValid;
    KFilePlacesModel *m_places = nullptr;
    KSharedConfig::Ptr m_config;
};
