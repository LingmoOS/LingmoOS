// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKUTIL_P_H
#define NETWORKUTIL_P_H

#include "global_defines.h"
#include "discover/deviceinfo.h"

#include <QObject>

class SessionManager;
namespace cooperation_core {

class NetworkUtil;
class NetworkUtilPrivate : public QObject
{
    friend class NetworkUtil;
    Q_OBJECT
public:
    explicit NetworkUtilPrivate(NetworkUtil *qq);
    ~NetworkUtilPrivate();

    QString getStorageFolder() const;

    QString getConfirmTargetAddress() const;

public Q_SLOTS:
    void handleConnectStatus(int result, QString reason);
    void handleTransChanged(int status, const QString &path, quint64 size);
    void handleAsyncRpcResult(int32_t type, const QString response);

private:
    NetworkUtil *q { nullptr };
    SessionManager *sessionManager { nullptr };

    QString confirmTargetAddress {""};   // remote ip address
    QString storageRoot = {};   //storage dir config
    QString storageFolder = {};   //sub folder under storage dir config
    int servePort;
};

}

#endif   // NETWORKUTIL_P_H
