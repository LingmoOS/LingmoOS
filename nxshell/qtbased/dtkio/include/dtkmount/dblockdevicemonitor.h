// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLOCKDEVICEMONITOR_H
#define DBLOCKDEVICEMONITOR_H

#include <DtkMountGlobal>

#include <QObject>

DMOUNT_BEGIN_NAMESPACE

class DBlockDeviceMonitor;
namespace DDeviceManager {
DBlockDeviceMonitor *globalBlockDeviceMonitor();
}

enum class BlockDeviceProperty;
class DBlockDeviceMonitorPrivate;
class DBlockDeviceMonitor : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DBlockDeviceMonitor)
    friend DBlockDeviceMonitor *DDeviceManager::globalBlockDeviceMonitor();

public:
    ~DBlockDeviceMonitor() override;

    void setWatchChanges(bool watchChanges);
    bool watchChanges() const;

Q_SIGNALS:
    void deviceAdded(const QString &devPath);
    void deviceRemoved(const QString &devPath);
    void mountAdded(const QString &devPath, const QString &mountPoint);
    void mountRemoved(const QString &devPath);
    void fileSystemAdded(const QString &devPath);
    void fileSystemRemoved(const QString &devPath);
    void driveAdded(const QString &drvPath);
    void driveRemoved(const QString &drvPath);
    void propertiesChanged(const QString &devPath, const QMap<Dtk::Mount::BlockDeviceProperty, QVariant> &changes);

private:
    explicit DBlockDeviceMonitor(QObject *parent = nullptr);
    QScopedPointer<DBlockDeviceMonitorPrivate> d_ptr;
};

DMOUNT_END_NAMESPACE

#endif   // DBLOCKDEVICEMONITOR_H
