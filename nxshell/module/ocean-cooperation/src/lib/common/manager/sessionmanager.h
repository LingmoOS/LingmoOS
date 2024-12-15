// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "sessionproto.h"
#include "common/exportglobal.h"

#include <QObject>

class SessionWorker;
class TransferWorker;
class FileSizeCounter;
class EXPORT_API SessionManager : public QObject
{
    Q_OBJECT
public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    void setSessionExtCallback(ExtenMessageHandler cb);
    void updatePin(QString code);
    void setStorageRoot(const QString &root);
    void updateSaveFolder(const QString &folder);
    void updateLoginStatus(QString &ip, bool logined);

    void sessionListen(int port);
    bool sessionPing(QString ip, int port);
    int sessionConnect(QString ip, int port, QString password);
    void sessionDisconnect(QString ip);
    void sendFiles(QString &ip, int port, QStringList paths);
    void recvFiles(QString &ip, int port, QString &token, QStringList names);
    void cancelSyncFile(const QString &ip, const QString &reason = "");

    void sendRpcRequest(const QString &target, int type, const QString &reqJson);

signals:
    void notifyCancelWeb();
    void notifyConnection(int result, QString reason);
    void notifyAsyncRpcResult(int32_t type, const QString response);

    // transfer status
    void notifyTransChanged(int status, const QString &path, quint64 size);

public slots:
    void handleTransData(const QString endpoint, const QStringList nameVector);
    void handleTransCount(const QString names, quint64 size);
    void handleCancelTrans(const QString jobid, const QString reason);
    void handleFileCounted(const QString ip, const QStringList paths, quint64 totalSize);
    void handleRpcResult(int32_t type, const QString &response);
    void handleTransException(const QString jobid, const QString reason);

private:
    std::shared_ptr<TransferWorker> createTransWorker(const QString &jobid);

private:
    // session worker
    std::shared_ptr<SessionWorker> _session_worker { nullptr };

    // new thread to count all sub files size of directory
    std::shared_ptr<FileSizeCounter> _file_counter { nullptr };

    // map to manage multiple transfer workers by jobid (IP address), it will be release after stop.
    std::map<QString, std::shared_ptr<TransferWorker>> _trans_workers;

    QString _save_root = "";
    QString _save_dir = "";
};

#endif // SESSIONMANAGER_H
