// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HANDLEIPDSERVICE_H
#define HANDLEIPDSERVICE_H

#include <QObject>
#include <QMap>
#include "co/json.h"
#include <CuteIPCService.h>

#include "common/commonstruct.h"

class HandleIpcService : public CuteIPCService
{
    Q_OBJECT
public:
    explicit HandleIpcService(QObject *parent = nullptr);
    ~HandleIpcService();

    Q_INVOKABLE QString bindSignal(const QString& appname, const QString& signalname);
    Q_INVOKABLE void registerDiscovery(bool unreg, const QString& name, const QString& info);
    Q_INVOKABLE QString getDiscovery();
    Q_INVOKABLE void saveAppConfig(const QString& name, const QString &key, const QString &value);

    Q_INVOKABLE void setAuthPassword(const QString& password);
    Q_INVOKABLE QString getAuthPassword();


    Q_INVOKABLE void sendMiscMessage(const QString& appname, const QString &jsonmsg);
    Q_INVOKABLE void doTryConnect(const QString& appname, const QString& targetname, const QString &host, const QString &pwd);

    Q_INVOKABLE void doTransferFile(const QString& session, const QString &targetsession, const int jobid,
                                    const QStringList paths, const bool hassub, const QString savedir);
    Q_INVOKABLE bool doOperateJob(const int action, const int jobid, const QString &appname);

    Q_INVOKABLE void doApplyTransfer(const QString &appname, const QString& targetname, const QString& machinename);
    Q_INVOKABLE void doReplyTransfer(const QString &appname, const QString& targetname, const QString& machinename, bool agree);

    Q_INVOKABLE void doApplyShare(const QString& appname, const QString &targetname, const QString &targetip, const QString &data);
    Q_INVOKABLE void doDisconnectShare(const QString& appname, const QString &targetname, const QString &msg);
    Q_INVOKABLE void doReplyShare(const QString& appname, const QString &targetname, const int reply);
    Q_INVOKABLE void doStartShare(const QString& appname, const QString &screenname);
    Q_INVOKABLE void doStopShare(const QString& appname, const QString &targetname, const int flags);
    Q_INVOKABLE void doDisconnectCallback(const QString& appname);
    Q_INVOKABLE void doCancelShareApply(const QString& appname);


    Q_INVOKABLE void doAsyncSearch(const QString &targetip, const bool remove);

    Q_INVOKABLE void appExit();

signals:
    void connectClosed(const quint16 port);
    // 使用这个信号必须是不需要等待客户端返回值的。
    void notifyConnect(const QString session, const QString ip, const QString pass);

    // for dde-cooperation app
    void cooperationSignal(int type, const QString& message);

    // for dde-transfer app
    // void ddeTransSignal(int type, const QString& message);

    // for data-transfer app
    void dataTransferSignal(int type, const QString& message);
public slots:
    void handleSessionSignal(const QString& signalName, int type, const QString& message);

    void newTransSendJob(QString session, const QString targetSession, int jobId, QStringList paths, bool sub, QString savedir);
    void handleNodeRegister(bool unreg, const co::Json &info);
    void handleBackApplyTransFiles(co::Json param);
    // void handleConnectClosed(const quint16 port);
    void handleTryConnect(co::Json json);
    bool handleJobActions(const uint type, co::Json &msg);
    void handleShareConnect(co::Json json);
    void handleShareDisConnect(co::Json json);
    void handleShareConnectReply(co::Json json);
    void handleShareConnectDisApply(co::Json json);
    void handleShareStop(co::Json json);
    void handleDisConnectCb(co::Json json);
    void handleShareServerStart(const bool ok, const QString msg);
    void handleSearchDevice(co::Json json);

private:
    QMap<QString, QString> _sessionIDs;
    // <appName, ip>
    QMap<QString, QString> _ips;
};

#endif // HANDLEIPDSERVICE_H
