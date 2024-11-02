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

#ifndef SYSTEMRESOURCEMANAGER_H
#define SYSTEMRESOURCEMANAGER_H

#include <QObject>

#include "resource.h"

class ResourceWatcher;
class SystemResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit SystemResourceManager(QObject *parent = nullptr);
    void startWatchers();
    QString reclaimProcesses(const QList<int> &pids) const;

Q_SIGNALS:
    void ResourceThresholdWarning(QString resource, int level);

private:
    void startWatcherThread(ResourceWatcher* watcher, QThread* thread) const;
    std::vector<ResourceWatcher*> createWatchers(
        const std::map<resource::Resource, resource::ResourceUrgencyThreshold>& resourceThresholds) const;
    std::map<resource::Resource, resource::ResourceUrgencyThreshold> parserResourceThreshold() const;
    std::string reclaimProcess(int pid) const;

};

#endif // SYSTEMRESOURCEMANAGER_H
