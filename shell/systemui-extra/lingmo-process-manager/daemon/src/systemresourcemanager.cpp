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

#include "systemresourcemanager.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include <jsoncpp/json/json.h>
#include <QThread>
#include <QDebug>

#include "pressurewatcher.h"
#include "memorywatcher.h"

static const char* RESOURCE_CONFIG_FILE = "/etc/lingmo-process-manager/lingmo-process-manager.json";
static const int RESOURCE_URGENCY_COUNT = 3;

using namespace resource;

SystemResourceManager::SystemResourceManager(QObject *parent)
    : QObject{parent}
{}

QString SystemResourceManager::reclaimProcesses(const QList<int> &pids) const
{
    qDebug() << "Reclaim processes:" << pids;

    if (pids.isEmpty()) {
        QString errorMessage = "Pids is empty.";
        return errorMessage;
    }
    if (access("/proc/1/reclaim", F_OK) == -1) {
        QString errorMessage = "Current system doesn't support memory reclamation.";
        return errorMessage;
    }

    for (const int& pid : qAsConst(pids)) {
        QString errorMessage = QString::fromStdString(reclaimProcess(pid));
        if (!errorMessage.isEmpty())
            return errorMessage;
    }

    return QString();
}

std::string SystemResourceManager::reclaimProcess(int pid) const
{
    const char* content = "all";
    std::string api = "/proc/" + std::to_string(pid) + "/reclaim";

    int fd = open(api.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        std::string errorMessage = "Open api file " + api + " failed, " + strerror(errno);
        return errorMessage;
    }
    if (write(fd, content, strlen(content) + 1) < 0) {
        close(fd);
        std::string errorMessage = "Write api file " + api + " failed, " + strerror(errno);
        return errorMessage;
    }

    return std::string();
}

std::vector<ResourceWatcher*> SystemResourceManager::createWatchers(
    const std::map<Resource, ResourceUrgencyThreshold>& resourceThresholds) const {
    std::vector<ResourceWatcher*> watchers;

    watchers.push_back(new PressureWatcher(resourceThresholds));
    watchers.push_back(new MemoryWatcher(resourceThresholds));

    return watchers;
}

void SystemResourceManager::startWatchers()
{
    std::map<Resource, ResourceUrgencyThreshold> resourceThresholds = parserResourceThreshold();
    std::vector<ResourceWatcher*> watchers = createWatchers(resourceThresholds);
    for (ResourceWatcher* watcher : watchers) {
        QThread* watcherThread = new QThread(this);
        startWatcherThread(watcher, watcherThread);
    }
}

void SystemResourceManager::startWatcherThread(ResourceWatcher* watcher, QThread* thread) const
{
    if (!watcher || !thread) {
        return;
    }

    watcher->moveToThread(thread);

    connect(thread, &QThread::started, watcher, &ResourceWatcher::start);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, watcher, &ResourceWatcher::deleteLater);
    connect(watcher, &ResourceWatcher::finished, thread, &QThread::quit);
    connect(watcher, &ResourceWatcher::ResourceThresholdWarning, this, &SystemResourceManager::ResourceThresholdWarning);

    thread->start();
}

std::map<Resource, ResourceUrgencyThreshold> SystemResourceManager::parserResourceThreshold() const
{
    std::map<Resource, ResourceUrgencyThreshold> resourceThresholdInfos;
    std::ifstream file(RESOURCE_CONFIG_FILE);
    Json::Value resourceConfig;

    if (!file.is_open()) {
        qWarning() << "Failed to open config file:" << RESOURCE_CONFIG_FILE;
        return resourceThresholdInfos;
    }
    file >> resourceConfig;

    resourceConfig = resourceConfig["ResourceThreshold"];
    for (Json::Value::ArrayIndex i = 0; i < resourceConfig.size(); ++i) {
        auto resourceThresholdInfo = resourceConfig[i];
        std::string resource = resourceThresholdInfo["resource"].asString();

        auto thresholds = resourceThresholdInfo["threshold"];
        if (RESOURCE_URGENCY_COUNT != thresholds.size()) {
            qWarning() << "Config file thresholds error, threshold count need:" << RESOURCE_URGENCY_COUNT
                       << "but current config threshold count is:" << thresholds.size();
            return resourceThresholdInfos;
        }

        ResourceUrgencyThreshold urgencyThreshold;
        for (Json::Value::ArrayIndex i = 0; i < thresholds.size(); ++i) {
            urgencyThreshold[intToResourceUrgencyEnum(i)] = thresholds[i].asInt();;
        }

        resourceThresholdInfos[stringToResourceEnum(resource)] = urgencyThreshold;
    }

    return resourceThresholdInfos;
}
