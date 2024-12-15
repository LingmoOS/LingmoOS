// SPDX-FileCopyrightText: 2023-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREHELPER_H
#define SHAREHELPER_H

#include "discover/deviceinfo.h"

namespace cooperation_core {

class ShareHelperPrivate;
class ShareHelper : public QObject
{
    Q_OBJECT

public:
    static ShareHelper *instance();

    void registConnectBtn();
    static void buttonClicked(const QString &id, const DeviceInfoPointer info);
    static bool buttonVisible(const QString &id, const DeviceInfoPointer info);

public Q_SLOTS:
    //server
    void connectToDevice(const DeviceInfoPointer info);
    void disconnectToDevice(const DeviceInfoPointer info);
    void notifyConnectRequest(const QString &info);
    void onVerifyTimeout();

    //client
    void handleConnectResult(int result, const QString &clientprint = "");
    void handleDisConnectResult(const QString &devName);
    void handleCancelCooperApply();
    void handleNetworkDismiss(const QString &msg);

    // exception: network connection(ping out) or other io
    void onShareExcepted(int type, const QString &remote);

    // check the self is sharing with someone
    int selfSharing(const QString &shareIp);

    //switch the client peripheral share by setting
    bool switchPeripheralShared(bool on);

private:
    explicit ShareHelper(QObject *parent = nullptr);
    ~ShareHelper();

    QSharedPointer<ShareHelperPrivate> d { nullptr };
};

}   // namespace cooperation_core

#endif   // SHAREHELPER_H
