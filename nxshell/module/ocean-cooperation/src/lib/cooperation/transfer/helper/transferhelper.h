// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERHELPER_H
#define TRANSFERHELPER_H

#include "global_defines.h"
#include "discover/deviceinfo.h"
#include "common/log.h"

namespace cooperation_core {
    class MainWindow;
}

namespace cooperation_transfer {

class TransferHelperPrivate;
class TransferHelper : public QObject
{
    Q_OBJECT

public:
    static TransferHelper *instance();

    void registBtn(cooperation_core::MainWindow *window);
    static void buttonClicked(const QString &id, const DeviceInfoPointer info);
    static bool buttonVisible(const QString &id, const DeviceInfoPointer info);
    static bool buttonClickable(const QString &id, const DeviceInfoPointer info);

signals:
    void onlineDevices(const QList<DeviceInfoPointer> &infoList);
    void offlineDevice(const QString &ip);
    void finishDiscovery(bool found);

    void refresh();
    void search(const QString &ip);
    void sendFiles(const QString &ip, const QString &devName, const QStringList &fileList);

public Q_SLOTS:
    void timeConnectBackend();
    void timeWatchBackend();

    // remote ipc callback slots
    void searchResultSlot(const QString& info);
    void refreshResultSlot(const QStringList& infoList);
    void deviceChangedSlot(bool found, const QString& info);

private:
    explicit TransferHelper(QObject *parent = nullptr);
    ~TransferHelper();

    DeviceInfoPointer parseFromJson(const QString &info);
    bool transable(const DeviceInfoPointer devInfo);
private:
    QSharedPointer<TransferHelperPrivate> d { nullptr };
#ifdef ENABLE_COMPAT
    QTimer *backendWatcher { nullptr };
#endif
};

}   // namespace cooperation_transfer

#endif   // TRANSFERHELPER_H
