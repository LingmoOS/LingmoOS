// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "handleipcservice.h"
#include "sendipcservice.h"
#include "service/rpc/sendrpcservice.h"
#include "service/share/sharecooperationservice.h"
#include "service/share/sharecooperationservicemanager.h"
#include "ipc/proto/chan.h"
#include "ipc/proto/comstruct.h"
#include "ipc/proto/backend.h"
#include "ipc/bridge.h"
#include "common/constant.h"
#include "common/commonstruct.h"
#include "service/comshare.h"
#include "service/discoveryjob.h"
#include "utils/config.h"
#include "service/jobmanager.h"
#include "protocol/version.h"

#include <QPointer>
#include <QCoreApplication>

HandleIpcService::HandleIpcService(QObject *parent)
    : CuteIPCService(parent)
{
}

HandleIpcService::~HandleIpcService()
{
}

QString HandleIpcService::bindSignal(const QString& appname, const QString& signalname)
{
    // gen new one
    QString sesid = co::randstr(appname.toStdString().c_str(), 8).c_str(); // 长度为8的16进制字符串
    _sessionIDs.insert(appname, sesid);
    // 创建新的sessionid
    SendIpcService::instance()->handleSaveSession(appname, sesid, signalname);
    return sesid;
}

void HandleIpcService::registerDiscovery(bool unreg, const QString& name, const QString& info)
{
    AppPeerInfo appPeer;
    appPeer.appname = name.toStdString();
    appPeer.json = info.toStdString();

    handleNodeRegister(unreg, appPeer.as_json());
}

QString HandleIpcService::getDiscovery()
{
    auto nodes = DiscoveryJob::instance()->getNodes();
    NodeList nodeInfos;
    for (const auto &node : nodes) {
        co::Json nodejs;
        nodejs.parse_from(node);
        NodeInfo info;
        info.from_json(nodejs);
        nodeInfos.peers.push_back(info);
    }
    auto _base = DiscoveryJob::instance()->baseInfo();
    co::Json _base_json;
    if (_base_json.parse_from(_base)) {
        NodePeerInfo _info;
        _info.from_json(_base_json);
        NodeInfo info;
        info.os = _info;
        nodeInfos.peers.push_back(info);
    }

    QString nodeStr = QString(nodeInfos.as_json().str().c_str());
    return nodeStr;
}

void HandleIpcService::saveAppConfig(const QString& name, const QString &key, const QString &value)
{
    DaemonConfig::instance()->setAppConfig(name.toStdString(), key.toStdString(), value.toStdString());
}

void HandleIpcService::setAuthPassword(const QString& password)
{
    if (password.isEmpty()) {
        //refresh as random password
        DaemonConfig::instance()->refreshPin();
    } else {
        fastring pwd = password.toStdString();
        DaemonConfig::instance()->setPin(pwd);
    }
}

QString HandleIpcService::getAuthPassword()
{
    QString pin = DaemonConfig::instance()->getPin().c_str();
    return pin;
}

void HandleIpcService::sendMiscMessage(const QString& appname, const QString &jsonmsg)
{
    MiscJsonCall misc;
    misc.app = appname.toStdString();
    misc.json = jsonmsg.toStdString();

    QString msg = misc.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(MISC, appname, msg);
}

void HandleIpcService::doTryConnect(const QString& appname, const QString& targetname, const QString &host, const QString &pwd)
{
    _ips.remove(appname);
    _ips.insert(appname, host);

    QString targetAppname = targetname.isEmpty() ? appname : targetname;
    QByteArray byteArray = pwd.toUtf8();
    UserLoginInfo login;

    // 使用base64加密auth
    login.name = appname.toStdString();
    login.auth = Util::encodeBase64(byteArray.data());

    std::string uuid = Util::genUUID();
    login.my_uid = uuid;
    login.my_name = Util::getHostname();
    login.selfappName = appname.toStdString();
    login.appName = targetAppname.toStdString();

    login.session_id = uuid;
    login.version = UNIAPI_VERSION;
    login.ip = Util::getFirstIp();
    LOG << " rcv client connet to " << host.toStdString() << appname.toStdString();

    QString jsonData = login.as_json().str().c_str();
    // 创建远程发送的work
    SendRpcService::instance()->createRpcSender(appname, host, UNI_RPC_PORT_BASE);
    SendRpcService::instance()->setTargetAppName(appname, targetAppname);
    SendRpcService::instance()->doSendProtoMsg(LOGIN_INFO, appname, jsonData);
}

void HandleIpcService::doTransferFile(const QString& session, const QString &targetsession, const int jobid,
                                const QStringList paths, const bool hassub, const QString savedir)
{
    newTransSendJob(session, targetsession, jobid, paths, hassub, savedir);
}

bool HandleIpcService::doOperateJob(const int action, const int jobid, const QString &appname)
{
    //action: TRANS_RESUME TRANS_CANCEL TRANS_PAUSE

    FileTransJobAction jobAction;
    jobAction.job_id = (jobid);
    jobAction.appname = (appname.toStdString());
    jobAction.type = (action);

    QString jsonData = jobAction.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(action, appname, jsonData);

    return JobManager::instance()->doJobAction(action, jobid);
}

void HandleIpcService::doApplyTransfer(const QString &appname, const QString& targetname, const QString& machinename)
{
    ApplyTransFiles transInfo;
    transInfo.appname = appname.toStdString();
    transInfo.type = ApplyTransType::APPLY_TRANS_APPLY;
    transInfo.tarAppname = targetname.toStdString();
    transInfo.machineName = machinename.toStdString();
    transInfo.selfIp = Util::getFirstIp();
    transInfo.selfPort = UNI_RPC_PORT_BASE;
                                                                                                                                                                               // 远程发送
    QString jsonData = transInfo.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(TRANS_APPLY, appname, jsonData);
}

void HandleIpcService::doReplyTransfer(const QString &appname, const QString& targetname, const QString& machinename, bool agree)
{
    ApplyTransFiles transInfo;
    transInfo.appname = appname.toStdString();
    transInfo.type = agree ? ApplyTransType::APPLY_TRANS_CONFIRM : ApplyTransType::APPLY_TRANS_REFUSED;;
    transInfo.tarAppname = targetname.toStdString();
    transInfo.machineName = machinename.toStdString();
    transInfo.selfIp = Util::getFirstIp();
    transInfo.selfPort = UNI_RPC_PORT_BASE;
                                            // 远程发送
    QString jsonData = transInfo.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(TRANS_APPLY, appname, jsonData);
}

void HandleIpcService::doApplyShare(const QString& appname, const QString &targetname, const QString &targetip, const QString &data)
{
    _ips.remove(appname);
    _ips.insert(appname, targetip);

    ShareConnectApply conEvent;
    conEvent.appName = appname.toStdString();
    conEvent.tarAppname = targetname.toStdString();
    conEvent.tarIp = targetip.toStdString();
    conEvent.ip = Util::getFirstIp();
    conEvent.data = data.toStdString();

    LOG << " rcv share connet to " << targetip.toStdString() << " "<< appname.toStdString();
    // 创建远程发送的work
    SendRpcService::instance()->createRpcSender(appname, targetip, UNI_RPC_PORT_BASE);
    // 发送给被控制端请求共享连接
    QString jsonData = conEvent.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_CONNECT, appname, jsonData);
    Comshare::instance()->updateStatus(CURRENT_STATUS_SHARE_CONNECT);
}

void HandleIpcService::doDisconnectShare(const QString& appname, const QString &targetname, const QString &msg)
{
    ShareDisConnect disConEvent;
    disConEvent.appName = appname.toStdString();
    disConEvent.tarAppname = targetname.toStdString();
    disConEvent.msg = msg.toStdString();

    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    DiscoveryJob::instance()->updateAnnouncShare(true);

    QString jsonData = disConEvent.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_DISCONNECT, appname, jsonData);
}

void HandleIpcService::doReplyShare(const QString& appname, const QString &targetname, const int reply)
{
    ShareConnectReply replyEvent;
    replyEvent.appName = appname.toStdString();
    replyEvent.tarAppname = targetname.toStdString();
    replyEvent.reply = reply;

    if (reply == SHARE_CONNECT_REFUSE)
        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    // 回复控制端连接结果
    QString jsonData = replyEvent.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_CONNECT_RES, appname, jsonData);
}

void HandleIpcService::doStartShare(const QString& appname, const QString &screenname)
{
    ShareServerConfig config;
    config.client_screen = screenname.toStdString(); // set client's name in server config.

    ShareStart startEvent;
    startEvent.appName = appname.toStdString();
    startEvent.config = config;

    startEvent.ip = Util::getFirstIp();
    startEvent.port = UNI_SHARE_SERVER_PORT;
    startEvent.tarAppname = appname.toStdString();

    Comshare::instance()->updateStatus(CURRENT_STATUS_SHARE_START);
    // 通知远端启动客户端连接到这里的barrier服务器
    SendRpcService::instance()->doSendProtoMsg(SHARE_START, appname,
                                               startEvent.as_json().str().c_str());
}

void HandleIpcService::doStopShare(const QString& appname, const QString &targetname, const int flags)
{
    // flag: SHARE_STOP_CLIENT SHARE_STOP_SERVER or SHARE_STOP_ALL
    ShareStop stopEvent;
    stopEvent.appName = appname.toStdString();
    stopEvent.tarAppname = targetname.toStdString();
    stopEvent.flags = flags;

    // 通知远端
    QString jsonData = stopEvent.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(SHARE_STOP, appname, jsonData);
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
}

void HandleIpcService::doDisconnectCallback(const QString& appname)
{
    ShareDisConnect info;
    info.appName = appname.toStdString();
    info.tarAppname = appname.toStdString();

    QString jsonData = info.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(DISCONNECT_CB, appname, jsonData);

    SendRpcService::instance()->removePing(appname);
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
}

void HandleIpcService::doCancelShareApply(const QString& appname)
{
    ShareConnectDisApply cancelEvent;
    cancelEvent.appName = appname.toStdString();
    cancelEvent.tarAppname = appname.toStdString();
    cancelEvent.ip = Util::getFirstIp();

    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);

    // 回复控制端连接结果
    QString jsonData = cancelEvent.as_json().str().c_str();
    SendRpcService::instance()->doSendProtoMsg(DISAPPLY_SHARE_CONNECT, appname, jsonData);
}

void HandleIpcService::doAsyncSearch(const QString &targetip, const bool remove)
{
    DLOG << "doAsyncSearch targetip = " << targetip.toStdString();
    DiscoveryJob::instance()->searchDeviceByIp(targetip, remove);
}

void HandleIpcService::appExit()
{
    DLOG << "client ask Exit!";
    qApp->exit(0);
}

void HandleIpcService::handleSessionSignal(const QString& signalName, int type, const QString& message)
{
    // send signal to frontend by bind sginal name
    QMetaObject::invokeMethod(this,
                              signalName.toLatin1(),
                              Qt::QueuedConnection,
                              Q_ARG(int, type),
                              Q_ARG(QString, message));
}


void HandleIpcService::newTransSendJob(QString session, const QString targetSession, int32 jobId, QStringList paths, bool sub, QString savedir)
{
    auto s = _sessionIDs.key(session);
    if (s.isEmpty()) {
        DLOG << "this session is invalid." << session.toStdString();
        return;
    }

    int32 id = jobId;
    fastring who = s.toStdString();
    fastring savepath = savedir.toStdString();

    co::Json pathjson;
    for (QString path : paths) {
        fastring filepath = path.toStdString();
        pathjson.push_back(filepath);
    }
    FileTransJob job;
    job.app_who = who;
    job.targetAppname = targetSession.toStdString();
    job.job_id = id;
    job.save_path = savepath;
    job.include_hidden = false;
    job.sub = sub;
    job.write = false;
    job.path = pathjson.str();
    job.ip = _ips.value(who.c_str()).toStdString();
    if (job.ip.empty()) {
        ELOG << "create trans file job to remote ip is empty, appName = " << who;
    }

    SendIpcService::instance()->handleAddJob(s, jobId);
    JobManager::instance()->handleRemoteRequestJob(job.as_json().str().c_str(), nullptr);
}

void HandleIpcService::handleNodeRegister(bool unreg, const co::Json &info)
{
    if (unreg) {
        AppPeerInfo appPeer;
        appPeer.from_json(info);
        fastring appname = appPeer.appname;
        // 移除ping
        SendRpcService::instance()->removePing(appname.c_str());
        SendIpcService::instance()->handleRemoveSessionByAppName(appname.c_str());
    }

    DiscoveryJob::instance()->updateAnnouncApp(unreg, info.str());
}

void HandleIpcService::handleBackApplyTransFiles(co::Json param)
{
     // 远程发送
    ApplyTransFiles info;
    info.from_json(param);
    info.selfIp = Util::getFirstIp();
    info.selfPort = UNI_RPC_PORT_BASE;
    SendRpcService::instance()->doSendProtoMsg(TRANS_APPLY,info.appname.c_str(), info.as_json().str().c_str());
}

// void HandleIpcService::handleConnectClosed(const quint16 port)
// {
//     // 不延时，还是可以ping通，资源还没有回收
//     QTimer::singleShot(1000, this, [port]{
//         SendIpcService::instance()->handleConnectClosed(port);
//     });
// }

void HandleIpcService::handleTryConnect(co::Json json)
{
    ipc::ConnectParam param;
    param.from_json(json);
    QString appName(param.appName.c_str());
    QString ip(param.host.c_str());
    _ips.remove(appName);
    _ips.insert(appName, ip);

    QString pass(param.password.c_str());
    QString targetAppname = param.targetAppname.empty() ? appName : param.targetAppname.c_str();
    UserLoginInfo login;

    // 使用base64加密auth
    login.name = param.appName;
    login.auth = Util::encodeBase64(param.password.c_str());

    std::string uuid = Util::genUUID();
    login.my_uid = uuid;
    login.my_name = Util::getHostname();
    login.selfappName = param.appName;
    login.appName = targetAppname.toStdString();

    login.session_id = uuid;
    login.version = UNIAPI_VERSION;
    login.ip = Util::getFirstIp();
    LOG << " rcv client connet to " << ip.toStdString() << appName.toStdString();
    // 创建远程发送的work
    SendRpcService::instance()->createRpcSender(param.appName.c_str(), ip, UNI_RPC_PORT_BASE);
    SendRpcService::instance()->setTargetAppName(appName, targetAppname);
    SendRpcService::instance()->doSendProtoMsg(LOGIN_INFO, appName, login.as_json().str().c_str());

}

bool HandleIpcService::handleJobActions(const uint type, co::Json &msg)
{
    ipc::TransJobParam param;
    param.from_json(msg);
    int jobid = param.job_id;
    uint action_type;
    QString appName(param.appname.c_str());
    if (BACK_RESUME_JOB == type) {
        action_type = TRANS_RESUME;
    } else if (BACK_CANCEL_JOB == type) {
        action_type = TRANS_CANCEL;
    } else {
        action_type = TRANS_PAUSE;
        DLOG << "unsupport job action: PAUSE.";
    }

    FileTransJobAction action;
    action.job_id = (jobid);
    action.appname = (param.appname);
    action.type = (type);

    //LOG << " send job action to " << appName.toStdString() <<  " type: " << action_type;
    SendRpcService::instance()->doSendProtoMsg(action_type, appName, action.as_json().str().c_str(), QByteArray());

    return JobManager::instance()->doJobAction(type, jobid);
}

void HandleIpcService::handleShareConnect(co::Json json)
{
    ShareConnectApply param;
    param.from_json(json);
    QString appName(param.appName.c_str());
    QString targetIp(param.tarIp.c_str());
    _ips.remove(appName);
    _ips.insert(appName, targetIp);
    QString targetAppname = param.tarAppname.empty() ? appName : param.tarAppname.c_str();

    param.ip = Util::getFirstIp();
    LOG << " rcv share connet to " << targetIp.toStdString() << appName.toStdString();
    // 创建远程发送的work
    SendRpcService::instance()->createRpcSender(appName, targetIp, UNI_RPC_PORT_BASE);
    // 发送给被控制端请求共享连接
    SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_CONNECT, appName, param.as_json().str().c_str());
    Comshare::instance()->updateStatus(CURRENT_STATUS_SHARE_CONNECT);
}

void HandleIpcService::handleShareDisConnect(co::Json json)
{
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    ShareDisConnect info;
    info.from_json(json);
    info.tarAppname = info.tarAppname.empty() ? info.appName : info.tarAppname;
    DiscoveryJob::instance()->updateAnnouncShare(true);
    SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_DISCONNECT, info.appName.c_str(),
                                               info.as_json().str().c_str());
}

void HandleIpcService::handleShareConnectReply(co::Json json)
{
    ShareConnectReply reply;
    reply.from_json(json);
    if (reply.reply == SHARE_CONNECT_REFUSE)
        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    // 回复控制端连接结果
    SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_CONNECT_RES,
                                               reply.appName.c_str(), json.str().c_str());
}
// 取消协同申请
void HandleIpcService::handleShareConnectDisApply(co::Json json)
{
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    ShareConnectDisApply reply;
    reply.from_json(json);

    reply.ip = Util::getFirstIp();
    // 回复控制端连接结果
    SendRpcService::instance()->doSendProtoMsg(DISAPPLY_SHARE_CONNECT,
                                               reply.appName.c_str(), json.str().c_str());

}

void HandleIpcService::handleShareStop(co::Json json)
{
    ShareStop st;
    st.from_json(json);

    // 通知远端
    SendRpcService::instance()->doSendProtoMsg(SHARE_STOP, st.appName.c_str(),
                                               st.as_json().str().c_str());
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
}

void HandleIpcService::handleDisConnectCb(co::Json json)
{
    ShareDisConnect info;
    info.from_json(json);
    info.tarAppname = info.tarAppname.empty() ? info.appName : info.tarAppname;
    SendRpcService::instance()->doSendProtoMsg(DISCONNECT_CB, info.tarAppname.c_str(),
                                               info.as_json().str().c_str());

    SendRpcService::instance()->removePing(info.tarAppname.c_str());
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
}

void HandleIpcService::handleShareServerStart(const bool ok, const QString msg)
{
    co::Json json;
    if (!json.parse_from(msg.toStdString())) {
        ELOG << "handleShareServerStart parse json error!!!!";
        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
        return;
    }
    ShareStart st;
    st.from_json(json);
    if (!ok) {
        // ShareEvents ev;
        // ev.eventType = FRONT_SHARE_START_REPLY;
        ShareStartReply reply;
        reply.result = false;
        reply.isRemote = false;
        reply.errorMsg = "init server error! param = " + json.str();
        // ev.data = reply.as_json().str();
        // auto req = ev.as_json();
        // // 通知前端
        // req.add_member("api", "Frontend.shareEvents");
        // SendIpcService::instance()->handleSendToClient(st.tarAppname.c_str(), req.str().c_str());

        // shareEvents
        QString jsonMsg = reply.as_json().str().c_str();
        emit cooperationSignal(FRONT_SHARE_START_REPLY, jsonMsg);

        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
        return;
    }
    Comshare::instance()->updateStatus(CURRENT_STATUS_SHARE_START);
    // 通知远端启动客户端连接到这里的batter服务器
    SendRpcService::instance()->doSendProtoMsg(SHARE_START, st.appName.c_str(),
                                               st.as_json().str().c_str());
}

void HandleIpcService::handleSearchDevice(co::Json json)
{
    UNIGO([json](){
        SearchDevice de;
        de.from_json(json);
        DiscoveryJob::instance()->searchDeviceByIp(de.ip.c_str(), de.remove);
    });
}
