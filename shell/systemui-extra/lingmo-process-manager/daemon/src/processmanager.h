/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QDBusContext>

#include "cgroupmanager.h"
#include "systemdunitmanager.h"
#include "systemresourcemanager.h"
#include "cpuraplenergymeter.h"

class ProcessManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit ProcessManager(QObject *parent = nullptr);

    bool CreateProcessGroup(const QString &groupPath, const QStringList &controllers, const QList<int> &pids);
    bool MoveProcessToGroup(const QString &groupPath, const QStringList &controllers, const QList<int> &pids);
    bool SetProcessGroupResourceLimit(const QString &groupPath, const QString &controller, const QString &attrFile, const QString &value);
    void RemoveProcessCGroup(const QString &groupPath);
    void SetSystemdUnitPropertyEnabled(const QString &unitName, const QString &propertyName, bool enabled);
    void ReclaimProcesses(const QList<int> &pids) const;
    void ReclaimProcessGroups(const QStringList &groupNames) const;
    double GetCpuEnergyConsumption() const;
    QList<int> GetGroupPids(const QString &groupName) const;

Q_SIGNALS:
    void ResourceThresholdWarning(QString resource, int level);

private:
    CGroupManager m_cgroupManager;
    SystemdUnitManager m_systemdUnitManager;
    SystemResourceManager m_systemResourceManager;
    CpuRaplEnergyMeter m_cpuEnergyMeter;
};

#endif // PROCESSMANAGER_H
