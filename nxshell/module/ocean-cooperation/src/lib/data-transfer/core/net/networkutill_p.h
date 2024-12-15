// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKUTIL_P_H
#define NETWORKUTIL_P_H

#include <QObject>

class SessionManager;
class NetworkUtil;
class NetworkUtilPrivate : public QObject
{
    struct TransferInfo
    {
        int64_t totalSize = 0;   // 总量
        int64_t transferSize = 0;   // 当前传输量
        int64_t maxTimeS = 0;   // 耗时

        void clear()
        {
            totalSize = 0;
            transferSize = 0;
            maxTimeS = 0;
        }
    };

    friend class NetworkUtil;
    Q_OBJECT
public:
    explicit NetworkUtilPrivate(NetworkUtil *qq);
    ~NetworkUtilPrivate();

    void updateTransProgress(uint64_t current, const QString &path);
public Q_SLOTS:
    void handleConnectStatus(int result, QString reason);
    void handleTransChanged(int status, const QString &path, quint64 size);
    void handleAsyncRpcResult(int32_t type, const QString response);

private:
    NetworkUtil *q { nullptr };
    SessionManager *sessionManager { nullptr };
    TransferInfo transferInfo;
    QString transferingFile;
    QString finishfile;
    QString confirmTargetAddress {};   // remote ip address
    QString storageFolder = {};   //sub folder under storage dir config
};

#endif   // NETWORKUTIL_P_H
