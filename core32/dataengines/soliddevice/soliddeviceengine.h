/*
    SPDX-FileCopyrightText: 2007 Christopher Blauvelt <cblauvelt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/predicate.h>

#include "devicesignalmapmanager.h"
#include "devicesignalmapper.h"
#include "hddtemp.h"
#include <KIO/FileSystemFreeSpaceJob>
#include <Plasma/DataEngine>
#include <Plasma/Service>

enum State {
    Idle = 0,
    Mounting = 1,
    Unmounting = 2,
};

enum OperationResult {
    Working = 0,
    Successful = 1,
    Unsuccessful = 2,
};

/**
 * This class evaluates the basic expressions given in the interface.
 */
class SolidDeviceEngine : public Plasma::DataEngine
{
    Q_OBJECT
    friend class SolidDeviceService;

public:
    SolidDeviceEngine(QObject *parent, const QVariantList &args);
    ~SolidDeviceEngine() override;
    Plasma::Service *serviceForSource(const QString &source) override;

protected:
    bool sourceRequestEvent(const QString &name) override;
    bool updateSourceEvent(const QString &source) override;

private:
    bool populateDeviceData(const QString &name);
    bool updateStorageSpace(const QString &udi);
    bool updateHardDiskTemperature(const QString &udi);
    bool updateEmblems(const QString &udi);
    bool updateInUse(const QString &udi);
    bool forceUpdateAccessibility(const QString &udi);
    void listenForNewDevices();

    // predicate in string form, list of devices by udi
    QMap<QString, QStringList> m_predicatemap;
    // udi, corresponding device
    QMap<QString, Solid::Device> m_devicemap;
    // udi, corresponding encrypted container udi;
    QMap<QString, QString> m_encryptedContainerMap;
    // path, for pending file system free space jobs
    QSet<QString> m_paths;
    DeviceSignalMapManager *m_signalmanager;

    HddTemp *m_temperature;
    Solid::DeviceNotifier *m_notifier;

private Q_SLOTS:
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
    void deviceChanged(const QString &udi, const QString &property, const QVariant &value);
    void sourceWasRemoved(const QString &source);
    void setMountingState(const QString &udi);
    void setUnmountingState(const QString &udi);
    void setIdleState(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void deviceChanged(const QMap<QString, int> &props);
};
