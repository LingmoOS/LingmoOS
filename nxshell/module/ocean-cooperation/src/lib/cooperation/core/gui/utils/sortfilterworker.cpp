// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortfilterworker.h"
#include "utils/historymanager.h"

using TransHistoryInfo = QMap<QString, QString>;
Q_GLOBAL_STATIC(TransHistoryInfo, transHistory)

using namespace cooperation_core;

SortFilterWorker::SortFilterWorker(QObject *parent)
    : QObject(parent)
{
    onTransHistoryUpdated();
    connect(HistoryManager::instance(), &HistoryManager::transHistoryUpdated, this, &SortFilterWorker::onTransHistoryUpdated, Qt::QueuedConnection);
}

void SortFilterWorker::stop()
{
    isStoped = true;
}

void SortFilterWorker::onTransHistoryUpdated()
{
    *transHistory = HistoryManager::instance()->getTransHistory();
}

int SortFilterWorker::calculateIndex(const QList<DeviceInfoPointer> &list, const DeviceInfoPointer info)
{
    int index = 0;
    switch (info->connectStatus()) {
    case DeviceInfo::Connected:
        // 连接中的设备放第一个
        index = 0;
        break;
    case DeviceInfo::Connectable: {
        index = findLast(list, DeviceInfo::Connectable, info);
        if (index != -1)
            break;

        index = findFirst(list, DeviceInfo::Offline);
        if (index != -1)
            break;

        index = list.size();
    } break;
    case DeviceInfo::Offline:
    default:
        index = list.size();
        break;
    }

    return index;
}

void SortFilterWorker::addDevice(const QList<DeviceInfoPointer> &infoList)
{
    if (isStoped)
        return;

    for (auto info : infoList) {
        if (isStoped)
            return;
        if (info->ipAddress() == selfip)
            continue;

        // 分别进行更新
        if (contains(allDeviceList, info)) {
            updateDevice(allDeviceList, info, false);
            if (contains(visibleDeviceList, info))
                updateDevice(visibleDeviceList, info, true);
            continue;
        }

        if (info->connectStatus() == DeviceInfo::Unknown)
            info->setConnectStatus(DeviceInfo::Connectable);

        auto index = calculateIndex(allDeviceList, info);
        allDeviceList.insert(index, info);

        // 判断是否需要过滤
        if (!filterText.isEmpty()) {
            if (info->deviceName().contains(filterText, Qt::CaseInsensitive)
                || info->ipAddress().contains(filterText, Qt::CaseInsensitive))
                index = calculateIndex(visibleDeviceList, info);
            else
                continue;
        }

        visibleDeviceList.insert(index, info);
        Q_EMIT sortFilterResult(index, info);
    }

    Q_EMIT filterFinished();
}

void SortFilterWorker::removeDevice(const QString &ip)
{
    for (int i = 0; i < visibleDeviceList.size(); ++i) {
        if (visibleDeviceList[i]->ipAddress() != ip)
            continue;

        allDeviceList.removeOne(visibleDeviceList[i]);
        visibleDeviceList.removeAt(i);
        Q_EMIT deviceRemoved(i);
        break;
    }
}

void SortFilterWorker::filterDevice(const QString &filter)
{
    filterText = filter;
    visibleDeviceList.clear();
    int index = -1;
    for (const auto &dev : allDeviceList) {
        if (dev->deviceName().contains(filter, Qt::CaseInsensitive)
            || dev->ipAddress().contains(filter, Qt::CaseInsensitive)) {
            ++index;
            visibleDeviceList.append(dev);
            Q_EMIT sortFilterResult(index, dev);
        }
    }

    Q_EMIT filterFinished();
}

void SortFilterWorker::clear()
{
    allDeviceList.clear();
    visibleDeviceList.clear();
}

int SortFilterWorker::findFirst(const QList<DeviceInfoPointer> &list, DeviceInfo::ConnectStatus state)
{
    int index = -1;
    auto iter = std::find_if(list.cbegin(), list.cend(),
                             [&](const DeviceInfoPointer info) {
                                 if (isStoped)
                                     return true;
                                 index++;
                                 return info->connectStatus() == state;
                             });

    if (iter == list.cend())
        return -1;

    return index;
}

int SortFilterWorker::findLast(const QList<DeviceInfoPointer> &list, DeviceInfo::ConnectStatus state, const DeviceInfoPointer info)
{
    bool isRecord = transHistory->contains(info->ipAddress());
    int startPos = -1;
    int endPos = -1;

    for (int i = list.size() - 1; i >= 0; --i) {
        if (list[i]->connectStatus() == state) {
            startPos = (startPos == -1 ? i : startPos);
            endPos = i;

            if (!isRecord)
                return startPos + 1;

            if (transHistory->contains(list[i]->ipAddress()))
                return endPos + 1;
        }
    }

    return qMin(startPos, endPos);
}

void SortFilterWorker::updateDevice(QList<DeviceInfoPointer> &list, const DeviceInfoPointer info, bool needNotify)
{
    int index = indexOf(list, info);

    // 当连接状态不一致时，需要更新位置
    bool needMove = list[index]->connectStatus() != info->connectStatus();
    if (needMove) {
        list.removeAt(index);
        auto to = calculateIndex(list, info);
        list.insert(to, info);
        if (needNotify)
            Q_EMIT deviceMoved(index, to, info);
    } else {
        list.replace(index, info);
        if (needNotify)
            Q_EMIT deviceUpdated(index, info);
    }
}

bool SortFilterWorker::contains(const QList<DeviceInfoPointer> &list, const DeviceInfoPointer info)
{
    auto iter = std::find_if(list.begin(), list.end(),
                             [&info](const DeviceInfoPointer it) {
                                 return it->ipAddress() == info->ipAddress();
                             });

    return iter != list.end();
}

int SortFilterWorker::indexOf(const QList<DeviceInfoPointer> &list, const DeviceInfoPointer info)
{
    int index = -1;
    auto iter = std::find_if(list.begin(), list.end(),
                             [&](const DeviceInfoPointer it) {
                                 index++;
                                 return it->ipAddress() == info->ipAddress();
                             });

    if (iter == list.end())
        return -1;

    return index;
}

void SortFilterWorker::setSelfip(const QString &value)
{
    selfip = value;
}
