// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTFILTERWORKER_H
#define SORTFILTERWORKER_H

#include "global_defines.h"
#include "discover/deviceinfo.h"

#include <QObject>

namespace cooperation_core {

class SortFilterWorker : public QObject
{
    Q_OBJECT
public:
    explicit SortFilterWorker(QObject *parent = nullptr);

    void stop();
    void setSelfip(const QString &value);

public Q_SLOTS:
    void addDevice(const QList<DeviceInfoPointer> &infoList);
    void removeDevice(const QString &ip);
    void filterDevice(const QString &filter);
    void clear();

Q_SIGNALS:
    void sortFilterResult(int index, const DeviceInfoPointer info);
    void deviceRemoved(int index);
    void deviceUpdated(int index, const DeviceInfoPointer info);
    void deviceMoved(int from, int to, const DeviceInfoPointer info);
    void filterFinished();

private Q_SLOTS:
    void onTransHistoryUpdated();

private:
    int calculateIndex(const QList<DeviceInfoPointer> &list, const DeviceInfoPointer info);
    int findFirst(const QList<DeviceInfoPointer> &list, DeviceInfo::ConnectStatus state);
    int findLast(const QList<DeviceInfoPointer> &list, DeviceInfo::ConnectStatus state, const DeviceInfoPointer info);
    void updateDevice(QList<DeviceInfoPointer> &list, const DeviceInfoPointer info, bool needNotify);
    bool contains(const QList<DeviceInfoPointer> &list, const DeviceInfoPointer info);
    int indexOf(const QList<DeviceInfoPointer> &list, const DeviceInfoPointer info);

private:
    QList<DeviceInfoPointer> visibleDeviceList;
    QList<DeviceInfoPointer> allDeviceList;
    QString filterText;
    QString selfip;
    std::atomic_bool isStoped { false };
};

}   // namespace cooperation_core

#endif   // SORTFILTERWORKER_H
