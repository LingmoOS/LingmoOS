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

#include "processmanager.h"

ProcessManager::ProcessManager(QObject *parent)
    : QObject{parent}
{
    connect(&m_systemResourceManager, &SystemResourceManager::ResourceThresholdWarning,
            this, &ProcessManager::ResourceThresholdWarning);
    m_systemResourceManager.startWatchers();
}

bool ProcessManager::CreateProcessGroup(const QString &groupPath, const QStringList &controllers, const QList<int> &pids)
{
    return m_cgroupManager.createProcessGroup(groupPath, controllers, pids);
}

bool ProcessManager::MoveProcessToGroup(const QString &groupPath, const QStringList &controllers, const QList<int> &pids)
{
    return m_cgroupManager.moveProcessToGroup(groupPath, controllers, pids);
}

bool ProcessManager::SetProcessGroupResourceLimit(const QString &groupPath, const QString &controller, const QString &attrFile, const QString &value)
{
    return m_cgroupManager.setProcessGroupResourceLimit(groupPath, controller, attrFile, value);
}

void ProcessManager::RemoveProcessCGroup(const QString &groupPath)
{
    m_cgroupManager.removeProcessCGroup(groupPath);
}

void ProcessManager::SetSystemdUnitPropertyEnabled(const QString &unitName, const QString &propertyName, bool enabled)
{
    m_systemdUnitManager.setSystemdUnitPropertyEnabled(unitName, propertyName, enabled);
}

void ProcessManager::ReclaimProcesses(const QList<int> &pids) const
{
    QString errorMessage = m_systemResourceManager.reclaimProcesses(pids);
    if (!errorMessage.isEmpty()) {
        sendErrorReply(QDBusError::ErrorType::Failed, errorMessage);
    }
}

void ProcessManager::ReclaimProcessGroups(const QStringList &groupNames) const
{
    QList<int> pids;
    for (const auto& groupName : groupNames) {
        pids << m_cgroupManager.getGroupPids(groupName);
    }

    QString errorMessage = m_systemResourceManager.reclaimProcesses(pids);
    if (!errorMessage.isEmpty()) {
        sendErrorReply(QDBusError::ErrorType::Failed, errorMessage);
    }
}

double ProcessManager::GetCpuEnergyConsumption() const
{
    if (!m_cpuEnergyMeter.isAvailable()) {
        sendErrorReply(QDBusError::ErrorType::NotSupported,
                       "The intel rapl interface is not supported.");
        return 0;
    }

    return m_cpuEnergyMeter.energyConsumption();
}

QList<int> ProcessManager::GetGroupPids(const QString &groupName) const
{
    return m_cgroupManager.getGroupPids(groupName);
}
