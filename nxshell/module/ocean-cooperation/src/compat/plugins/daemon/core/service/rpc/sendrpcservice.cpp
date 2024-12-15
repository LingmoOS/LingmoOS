// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sendrpcservice.h"
#include "common/constant.h"
#include "ipc/proto/comstruct.h"
#include "ipc/proto/chan.h"
#include "ipc/proto/backend.h"
#include "ipc/bridge.h"
#include "service/comshare.h"
#include "service/ipc/sendipcservice.h"
#include "utils/utils.h"
#include "service/jobmanager.h"
#include "common/commonstruct.h"
#include "service/discoveryjob.h"

#include <QCoreApplication>

SendRpcWork::~SendRpcWork()
{

}

SendRpcWork::SendRpcWork(QObject *parent) : QObject (parent)
{
}

void SendRpcWork::handleCreateRpcSender(const QString appName, const QString targetip, quint16 port)
{
    if (_stoped)
        return;
    createRpcSender(appName, targetip, port);
}

void SendRpcWork::handleSetTargetAppName(const QString appName, const QString targetAppName)
{
    if (_stoped)
        return;
    auto sender = this->rpcSender(appName);
    if (sender.isNull())
        return;
    sender->setTargetAppName(targetAppName);
}

void SendRpcWork::handleDoSendProtoMsg(const uint32 type, const QString appName, const QString msg, const QByteArray data)
{
    if (_stoped)
        return;
    DLOG << "000000 " << appName.toStdString() << type << msg.toStdString();
    auto sender = this->rpcSender(appName);

    SendResult res;
    if (!sender.isNull()) {
        if (type == TRANS_APPLY) {
            co::Json param;
            param.parse_from(msg.toStdString());
            ApplyTransFiles info;
            info.from_json(param);
            if (info.type != APPLY_TRANS_APPLY) {
                info.appname = appName.toStdString();

                QString tar = sender->targetAppname();
                info.tarAppname = tar.isEmpty() ?
                                  appName.toStdString() : tar.toStdString();
            }
            res = sender->doSendProtoMsg(type, info.as_json().str().c_str(), data);
        } else if (type == APPLY_SHARE_CONNECT_RES) {
            co::Json param;
            param.parse_from(msg.toStdString());
            ShareConnectReply info;
            info.from_json(param);
            QString tar = sender->targetAppname();
            // 同意
            if (info.reply == 1) {
                auto _tarip = sender->remoteIP();
                info.ip = Util::getFirstIp();
                DiscoveryJob::instance()->updateAnnouncShare(false, _tarip.toStdString());
            }
            info.tarAppname = tar.isEmpty() ?
                              appName.toStdString() : tar.toStdString();
            res = sender->doSendProtoMsg(type, info.as_json().str().c_str(), data);
        } else {
            res = sender->doSendProtoMsg(type, msg, data);
        }

    } else {
        SendResult res;
        res.protocolType = type;
        res.errorType = PARAM_ERROR;
        res.data = "There is no remote sender!!!!!!";
    }
    if (_stoped)
        return;
    // todo 发送信号给其他使用模块
    emit sendToRpcResult(appName, res.as_json().str().c_str());
}

void SendRpcWork::handlePing(const QStringList apps)
{
    if (_stoped)
        return;

    for (const auto &appName : apps) {
        if (_stoped)
            return;
        auto sender = this->rpcSender(appName);
        if (sender.isNull())
            continue;

        PingPong ping;
        ping.appName = appName.toStdString();
        ping.tarAppname = sender->targetAppname().toStdString();
        ping.ip = Util::getFirstIp();

        SendResult rs = sender->doSendProtoMsg(RPC_PING, ping.as_json().str().c_str(), QByteArray());
        if (rs.data.empty() || rs.errorType < INVOKE_OK) {
            DLOG << "remote server no reply ping !!!!! " << appName.toStdString();
            auto count = _ping_failed_count.take(appName);
            if (count > 2) {
                // 通知客户端ping超时
                ELOG << "timeout: server no reply ping: " << count;
                fastring msg = co::Json({{"app", appName.toStdString()}, {"offline", true}}).str();
                SendIpcService::instance()->preprocessOfflineStatus(appName, PING_FAILED, msg);
                SendRpcService::instance()->removePing(appName);
            } else {
                _ping_failed_count.insert(appName, ++count);
            }
        } else {
            // 取消离线预处理消息
            SendIpcService::instance()->cancelOfflineStatus(appName);
            _ping_failed_count.remove(appName);
            _ping_failed_count.insert(appName, 0);
        }
    }
}

QSharedPointer<RemoteServiceSender> SendRpcWork::createRpcSender(const QString &appName,
                                                                 const QString &targetip, uint16_t port)
{
    if (_remotes.contains(targetip)) {
        _app_ips.remove(appName);
        _app_ips.insert(appName, targetip);
        return _remotes.value(targetip);
    }

    auto ip = _app_ips.value(appName);
    _app_ips.remove(appName);
    _app_ips.insert(appName, targetip);
    QSharedPointer<RemoteServiceSender> remote(new RemoteServiceSender(appName, targetip, port, false));
    _remotes.insert(targetip, remote);

    if (!ip.isEmpty() && _app_ips.keys(ip).isEmpty())
        _remotes.remove(ip);

    return remote;
}

QSharedPointer<RemoteServiceSender> SendRpcWork::rpcSender(const QString &appName)
{
    // 获取ip
    auto ip = _app_ips.value(appName);
    if (ip.isEmpty()) {
        ELOG_IF(FLG_log_detail) << "has no ip, appname = " << appName.toStdString();
        return nullptr;
    }
    // 获取 remote
    auto remote = _remotes.value(ip);
    if (remote.isNull()) {
        remote.reset(new RemoteServiceSender(appName, ip, UNI_RPC_PORT_BASE, false));
        _remotes.insert(ip, remote);
    }
    return remote;
}

SendRpcService::SendRpcService(QObject *parent)
    : QObject(parent)
{

    initConnet();
}

void SendRpcService::initConnet()
{
    _ping_timer.setInterval(1000);
    _work.moveToThread(&_thread);

    connect(&_ping_timer, &QTimer::timeout, this, &SendRpcService::handleTimeOut);
    connect(this, &SendRpcService::startPingTimer, this, &SendRpcService::handleStartTimer, Qt::QueuedConnection);
    connect(this, &SendRpcService::stopPingTimer, this, &SendRpcService::handleStopTimer, Qt::QueuedConnection);

    connect(&_work, &SendRpcWork::sendToRpcResult, this, &SendRpcService::sendToRpcResult, Qt::QueuedConnection);
    connect(this, &SendRpcService::workCreateRpcSender, &_work, &SendRpcWork::handleCreateRpcSender, Qt::QueuedConnection);
    connect(this, &SendRpcService::workSetTargetAppName, &_work, &SendRpcWork::handleSetTargetAppName, Qt::QueuedConnection);
    connect(this, &SendRpcService::ping, &_work, &SendRpcWork::handlePing, Qt::QueuedConnection);
    connect(this, &SendRpcService::workDoSendProtoMsg, &_work, &SendRpcWork::handleDoSendProtoMsg, Qt::QueuedConnection);
    _thread.start();
}

SendRpcService::~SendRpcService()
{
    handleAboutQuit();
}

SendRpcService *SendRpcService::instance()
{
    static SendRpcService service;
    return &service;
}

void SendRpcService::removePing(const QString &appName)
{
    QWriteLocker lk(&_ping_lock);
    _ping_appname.removeOne(appName);
    if (_ping_appname.isEmpty())
        emit stopPingTimer();
}

void SendRpcService::addPing(const QString &appName)
{
    QWriteLocker lk(&_ping_lock);
    if (!_ping_appname.contains(appName))
        _ping_appname.append(appName);
    if (!_ping_timer.isActive())
        emit startPingTimer();
}

void SendRpcService::handleStartTimer()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    if (!_ping_timer.isActive())
        _ping_timer.start();
}

void SendRpcService::handleStopTimer()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    _ping_timer.stop();
}

void SendRpcService::handleTimeOut()
{
    QReadLocker lk(&_ping_lock);
    emit ping(_ping_appname);
}

void SendRpcService::handleAboutQuit()
{
    _work.stop();
    _thread.quit();
    _thread.wait(3000);
    _thread.exit();
}
