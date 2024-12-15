// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "handlerpcservice.h"
#include "service/ipc/sendipcservice.h"
#include "sendrpcservice.h"
#include "service/rpc/remoteservice.h"
#include "common/constant.h"
#include "common/commonstruct.h"
#include "service/comshare.h"
#include "service/discoveryjob.h"
#include "ipc/proto/comstruct.h"
#include "ipc/bridge.h"
#include "service/jobmanager.h"
#include "protocol/version.h"
#include "utils/config.h"
#include "service/fsadapter.h"

#include "utils/cert.h"

#include <QPointer>
#include <QCoreApplication>

co::chan<IncomeData> _income_chan(10, 300);
co::chan<OutData> _outgo_chan(10, 20);
HandleRpcService::HandleRpcService(QObject *parent)
    : QObject(parent)
{
    _rpc.reset(new RemoteServiceBinder);
    _rpc_trans.reset(new RemoteServiceBinder);
    _timeOut.setInterval(1000);
    connect(&_timeOut, &QTimer::timeout, this, &HandleRpcService::handleTimeOut);
    connect(this, &HandleRpcService::startTimer, this, &HandleRpcService::handleStartTimer,
            Qt::QueuedConnection);
}

HandleRpcService::~HandleRpcService()
{
}

void HandleRpcService::startRemoteServer()
{
    startRemoteServer(UNI_RPC_PORT_BASE);
    startRemoteServer(UNI_RPC_PORT_TRANS);
}

void HandleRpcService::handleRpcLogin(bool result, const QString &targetAppname,
                                      const QString &appName, const QString &ip)
{
    // todo拿到客户端ip，进行消息通讯
    if (result) {
        SendRpcService::instance()->createRpcSender(appName, ip, UNI_RPC_PORT_BASE);
        SendRpcService::instance()->setTargetAppName(appName, targetAppname);
        // 设置为文件传输的连接状态
        Comshare::instance()->updateStatus(CURRENT_STATUS_TRAN_CONNECT);
        Comshare::instance()->updateComdata(appName, targetAppname, ip);
        // 启动文件的监视

        // 接受登陆，开始监听连接是否断开（来自对方的ping）
        if (!_timeOut.isActive())
            emit startTimer();
    } else {
        QWriteLocker lk(&_lock);
        _ping_lost_count.remove(appName);
    }

    //cbConnect {GenericResult}
    co::Json req = {
        { "id", 0 },
        { "result", result ? 1 : 0 },
        { "msg", (ip + " " + appName).toStdString() },
        { "isself", false},
    };

    QString jsonData = req.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_CONNECT_CB, jsonData);
}

bool HandleRpcService::handleRemoteApplyTransFile(co::Json &info)
{
    ApplyTransFiles obj;
    obj.from_json(info);

    {
        QWriteLocker lk(&_lock);
        auto app = QString(obj.tarAppname.c_str());
        _ping_lost_count.remove(app);
    }
#ifdef __linux__
    // for regist daemon-cooperation default.
    auto tmp = obj.tarAppname;
    obj.tarAppname = obj.appname;
    obj.appname = tmp;
#endif

    //notifyFileStatus {FileStatus}
    auto targetAppname = QString(obj.appname.c_str());
    QString jsonData = obj.as_json().str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_APPLY_TRANS_FILE, jsonData);
    if (obj.type != APPLY_TRANS_APPLY)
        SendRpcService::instance()->removePing(targetAppname);

    return true;
}

bool HandleRpcService::handleRemoteLogin(co::Json &info)
{
    UserLoginInfo lo;
    lo.from_json(info);
    UserLoginResultInfo lores;

    std::string version = lo.version.c_str();
    if (version.compare(UNIAPI_VERSION) != 0) {
        // Notification not match version
        lores.result = false;
        lores.token = "Invalid version";
    } else {
        bool authOK = false;

        fastring pwd = lo.auth;
        if (pwd.empty()) {
            // TODO: 无认证，用户确认
            if (DaemonConfig::instance()->needConfirm()) {
                // 目前还没有处理
            } else {
                authOK = true;
            }
        } else {
            fastring pass = Util::decodeBase64(pwd.c_str());
            //            LOG << "pass= " << pass << " getPin=" << DaemonConfig::instance()->getPin();
            authOK = DaemonConfig::instance()->getPin().compare(pass) == 0;
        }
        bool statu = true;//Comshare::instance()->checkTransCanConnect();
        if (!authOK || !statu) {
            lores.result = false;
            lores.token = authOK ? "connect file status error!" : "Invalid auth code";
        } else {
            DaemonConfig::instance()->saveRemoteSession(lo.session_id);

            //TODO: generate auth token
            fastring auth_token = "thatsgood";
            DaemonConfig::instance()->setTargetName(lo.my_name.c_str());   // save the login name
            fastring plattsr;
            if (WINDOWS == Util::getOSType()) {
                plattsr = "Windows";
            } else {
                //TODO: other OS
                plattsr = "UOS";
            }

            lores.peer.version = version;
            lores.peer.hostname = Util::getHostname();
            lores.peer.platform = plattsr.c_str();
            lores.peer.username = Util::getUsername();
            lores.token = auth_token;
            lores.appName = lo.selfappName;
            lores.result = authOK;
        }

        UserLoginResult result;
        result.appname = lo.appName;
        result.uuid = lo.my_uid;
        result.ip = lo.ip;
        result.result = authOK;
        QString appname(result.appname.c_str());
        handleRpcLogin(result.result, lo.selfappName.c_str(), appname, result.ip.c_str());
    }
    OutData data;
    data.type = LOGIN_INFO;
    data.json = lores.as_json().str();
    _outgo_chan << data;

    return true;
}

void HandleRpcService::handleRemoteDisc(co::Json &info)
{
    DLOG_IF(FLG_log_detail) << "handleRemoteDisc: " << info.dbg();
    MiscJsonCall mis;
    mis.from_json(info);

    auto targetAppname = QString(mis.app.c_str());
    QString jsonData = mis.json.c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, MISC_MSG, jsonData);
}

void HandleRpcService::handleRemoteFileBlock(co::Json &info, fastring data)
{
    FileTransResponse reply;
    auto res = JobManager::instance()->handleFSData(info, data, &reply);

    OutData out;
    out.type = FS_DATA;
    reply.result = (res ? OK : IO_ERROR);
    out.json = reply.as_json().str();
    _outgo_chan << out;
}

void HandleRpcService::handleRemoteReport(co::Json &info)
{
    FileTransResponse reply;
    reply.result = OK;
    OutData out;
    out.type = FS_REPORT;

    JobManager::instance()->handleTransReport(info, &reply);
    out.json = reply.as_json().str();
    _outgo_chan << out;
}

void HandleRpcService::handleRemoteJobCancel(co::Json &info)
{
    FileTransResponse reply;
    reply.result = OK;
    OutData out;
    out.type = TRANS_CANCEL;

    JobManager::instance()->handleCancelJob(info, &reply);
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    out.json = reply.as_json().str();
    _outgo_chan << out;
}

void HandleRpcService::handleTransJob(co::Json &info)
{
    QString app;
    bool res = false;
    res = JobManager::instance()->handleRemoteRequestJob(info.str().c_str(), &app);
    if (res) {
        Comshare::instance()->updateStatus(CURRENT_STATUS_TRAN_FILE_RCV);
    }

    if (!app.isEmpty()) {
        // 发送端已移除ping, 这里接收端始终也移除
        QWriteLocker lk(&_lock);
        _ping_lost_count.remove(app);
    }

    FileTransResponse reply;
    reply.result = (res ? OK : IO_ERROR);

    OutData data;
    data.type = TRANSJOB;
    data.json = reply.as_json().str();
    _outgo_chan << data;
}

void HandleRpcService::handleRemoteShareConnect(co::Json &info)
{
    ShareConnectApply lo;
    lo.from_json(info);

    // 这是收到的appname是对方的appname
    SendRpcService::instance()->createRpcSender(lo.tarAppname.c_str(), lo.ip.c_str(),
                                                UNI_RPC_PORT_BASE);
    SendRpcService::instance()->setTargetAppName(lo.tarAppname.c_str(),
                                                 lo.appName.c_str());
    // 判断当前是否连接
    co::Json baseJson;
    if (baseJson.parse_from(DiscoveryJob::instance()->baseInfo())) {
        //NodeInfo
        NodePeerInfo nodepeer;
        nodepeer.from_json(baseJson);
        if (!nodepeer.share_connect_ip.empty()
                && nodepeer.share_connect_ip.compare(nodepeer.ipv4) != 0) {
            ShareConnectReply reply;
            reply.ip = nodepeer.ipv4;
            reply.appName = lo.appName;
            reply.tarAppname = lo.tarAppname;
            reply.reply = SHARE_CONNECT_ERR_CONNECTED;
            // 回复控制端连接结果
            SendRpcService::instance()->doSendProtoMsg(APPLY_SHARE_CONNECT_RES,
                                                       reply.appName.c_str(), reply.as_json().str().c_str());
            return;
        }
    }
    Comshare::instance()->updateStatus(CURRENT_STATUS_SHARE_CONNECT);
    // shareEvents
    auto targetAppname = QString(lo.tarAppname.c_str());
    QString jsonMsg = info.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SHARE_APPLY_CONNECT, jsonMsg);
}

void HandleRpcService::handleRemoteShareDisConnect(co::Json &info)
{
    // 发送给前端
    ShareDisConnect sd;
    sd.from_json(info);
    DiscoveryJob::instance()->updateAnnouncShare(true);
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);

    // shareEvents
    auto targetAppname = QString(sd.tarAppname.c_str());
    QString jsonMsg = info.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SHARE_DISCONNECT, jsonMsg);
}

void HandleRpcService::handleRemoteShareConnectReply(co::Json &info)
{
    ShareConnectReply reply;
    reply.from_json(info);

    if (reply.reply == SHARE_CONNECT_COMFIRM) {
        DiscoveryJob::instance()->updateAnnouncShare(false, reply.ip);
    } else {
        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    }

    // shareEvents
    auto targetAppname = QString(reply.tarAppname.c_str());
    QString jsonMsg = info.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SHARE_APPLY_CONNECT_REPLY, jsonMsg);
}

void HandleRpcService::handleRemoteShareStart(co::Json &info)
{
    Comshare::instance()->updateStatus(CURRENT_STATUS_SHARE_START);
    ShareStart st;
    st.from_json(info);
    ShareEvents evs;
    evs.eventType = SHARE_START_RES;

    ShareStartReply reply;
    reply.result = true;
    reply.isRemote = false;

    if (!reply.result)
        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    // 通知远程
    SendRpcService::instance()->doSendProtoMsg(SHARE_START_RES, st.tarAppname.c_str(),
                                               evs.as_json().str().c_str());

    // 通知前端 shareEvents
    auto targetAppname = QString(st.tarAppname.c_str());
    QString jsonMsg = reply.as_json().str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SHARE_START_REPLY, jsonMsg);
}

void HandleRpcService::handleRemoteShareStartRes(co::Json &info)
{
    ShareStartRmoteReply rreply;
    rreply.from_json(info);
    ShareStartReply reply;
    reply.result = rreply.result;
    reply.isRemote = true;
    reply.errorMsg = rreply.errorMsg;

    if (!reply.result)
        Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);

    // 通知前端 shareEvents
    auto targetAppname = QString(rreply.tarAppname.c_str());
    QString jsonMsg = reply.as_json().str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, SHARE_START_RES, jsonMsg);
}

void HandleRpcService::handleRemoteShareStop(co::Json &info)
{
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    ShareStop st;
    st.from_json(info);

    // shareEvents
    auto targetAppname = QString(st.tarAppname.c_str());
    QString jsonMsg = info.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SHARE_STOP, jsonMsg);
}

void HandleRpcService::handleRemoteDisConnectCb(co::Json &info)
{
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    // 发送给前端
    ShareDisConnect sd;
    sd.from_json(info);

    //cbDisConnect
    auto targetAppname = QString(sd.tarAppname.c_str());
    QString jsonMsg = info.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_DISCONNECT_CB, jsonMsg);

    SendRpcService::instance()->removePing(sd.tarAppname.c_str());
}

void HandleRpcService::handleRemotePing(co::Json &info)
{
    PingPong pingjson;
    pingjson.from_json(info);
    if (QString(pingjson.ip.c_str()).contains("search-ping"))
        return;
    auto appName = QString(pingjson.appName.c_str());
    auto remoteip = QString(pingjson.ip.c_str());
    if (!_timeOut.isActive())
        emit startTimer();
    QWriteLocker lk(&_lock);
    _ping_lost_count.remove(appName);
    _ping_lost_count.insert(appName, 0);
    // 记录远端ping的IP地址，断开连接时通过ip获取要通知的Appname
    _remote_ping_ips.remove(appName);
    _remote_ping_ips.insert(appName, remoteip);

    //取消离线预处理
    SendIpcService::instance()->cancelOfflineStatus(appName);
}

void HandleRpcService::handleRemoteDisApplyShareConnect(co::Json &info)
{
    Comshare::instance()->updateStatus(CURRENT_STATUS_DISCONNECT);
    // 发送给前端
    ShareConnectDisApply sd;
    sd.from_json(info);

    // ShareEvents event;
    auto targetAppname = QString(sd.tarAppname.c_str());
    QString jsonMsg = info.str().c_str();
    SendIpcService::instance()->handleSendToClient(targetAppname, FRONT_SHARE_DISAPPLY_CONNECT, jsonMsg);

    SendRpcService::instance()->removePing(sd.tarAppname.c_str());
}

bool HandleRpcService::checkConnected()
{
    return _rpc->checkConneted() || _rpc_trans->checkConneted();
}

void HandleRpcService::handleRemoteSearchIp(co::Json &info)
{
    Q_UNUSED(info);
    OutData data;
    data.type = SEARCH_DEVICE_BY_IP;
    data.json = DiscoveryJob::instance()->nodeInfoStr();
    _outgo_chan << data;
}

void HandleRpcService::hanldeRemoteDiscover(co::Json &info)
{
    DiscoverInfo dis, res;
    OutData data;
    data.type = DISCOVER_BY_TCP;
    res.ip = Util::getFirstIp();
    res.msg = DiscoveryJob::instance()->udpSendPackage();
    data.json = res.as_json().str();
    _outgo_chan << data;

    dis.from_json(info);
    DiscoveryJob::instance()->handleUpdPackage(dis.ip.c_str(), dis.msg.c_str());
}

void HandleRpcService::handleOffline(const QString ip)
{
    QWriteLocker lk(&_lock);
    for (auto it = _remote_ping_ips.begin(); it != _remote_ping_ips.end();) {
        QString remoteip = it.value();
        if (ip.compare(remoteip) == 0) {
            QString appname = it.key();
            fastring msg = co::Json({{"ip", remoteip.toStdString()}, {"appName", appname.toStdString()}}).str();
            ELOG << "connection offline: " << appname.toStdString() << " ip:" << remoteip.toStdString();
            SendIpcService::instance()->preprocessOfflineStatus(appname, CONNCB_TIMEOUT, msg);
            // 移除ping记录
            it = _remote_ping_ips.erase(it);
            continue;
        }
        it++;
    }
}

void HandleRpcService::startRemoteServer(const quint16 port)
{
    if (_rpc.isNull() && port != UNI_RPC_PORT_TRANS)
        return;
    if (_rpc_trans.isNull() && port == UNI_RPC_PORT_TRANS)
        return;
    auto rpc = port != UNI_RPC_PORT_TRANS ? _rpc : _rpc_trans;
    fastring key = Cert::instance()->writeKey();
    fastring crt = Cert::instance()->writeCrt();
    QPointer<HandleRpcService> my = this;
    auto callback = [my](const int type, const fastring &ip, const uint16 port){
        if (type < 0) {
            ELOG << "connection callback offline: " << ip << ":" << port;
            if (!my.isNull()) {
                QString remoteip = QString(ip.c_str());
                my->handleOffline(remoteip);
            }
        }
    };
    if (port == UNI_RPC_PORT_TRANS) {
        rpc->startRpcListen(key.c_str(), crt.c_str(), port, callback);
    } else {
        rpc->startRpcListen(key.c_str(), crt.c_str(), port);
    }
    Cert::instance()->removeFile(key);
    Cert::instance()->removeFile(crt);

    QPointer<HandleRpcService> self = this;
    UNIGO([self]() {
        // 这里已经是线程或者协程
        while (!self.isNull()) {
            IncomeData indata;
            _income_chan >> indata;
            if (!_income_chan.done()) {
                // timeout, next read
                continue;
            }
            LOG_IF(FLG_log_detail) << ">> get chan value: " << indata.type << " json:" << indata.json;
            co::Json json_obj = json::parse(indata.json);
            if (json_obj.is_null()) {
                ELOG << "parse error from: " << indata.json;
                continue;
            }
            switch (indata.type) {
            case LOGIN_INFO:
            {
                self->handleRemoteLogin(json_obj);
                break;
            }
            case LOGIN_CONFIRM:
            {
                //TODO: notify user confirm login
                break;
            }
            case LOGIN_RESULT:// 服务器端回复登陆结果
            {
                break;
            }
            case TRANSJOB:
            {
                self->handleTransJob(json_obj);
                break;
            }
            case FS_DATA:
            {
                // must update the binrary data into struct object.
                self->handleRemoteFileBlock(json_obj, indata.buf);
                break;
            }
            case TRANS_CANCEL:
            {
                self->handleRemoteJobCancel(json_obj);
                break;
            }
            case FS_REPORT:
            {
                self->handleRemoteReport(json_obj);
                break;
            }
            case TRANS_APPLY:
            {
                OutData data;
                data.type = TRANS_APPLY;
                _outgo_chan << data;
                self->handleRemoteApplyTransFile(json_obj);
                break;
            }
            case MISC:
            {
                OutData data;
                data.type = MISC;
                _outgo_chan << data;
                self->handleRemoteDisc(json_obj);
                break;
            }
            case RPC_PING:
            {
                PingPong pong;
                pong.ip = Util::getFirstIp();
                OutData data;
                data.type = RPC_PING;
                data.json = pong.as_json().str();
                _outgo_chan << data;
                if (self)
                    self->handleRemotePing(json_obj);
                break;
            }
            case APPLY_SHARE_CONNECT:
            {
                // 被控制方收到共享连接申请
                OutData data;
                data.type = APPLY_SHARE_CONNECT;
                _outgo_chan << data;
                self->handleRemoteShareConnect(json_obj);
                break;
            }
            case APPLY_SHARE_DISCONNECT: {
                // 被控制方收到共享断开连接申请
                OutData data;
                data.type = APPLY_SHARE_DISCONNECT;
                _outgo_chan << data;
                self->handleRemoteShareDisConnect(json_obj);
                break;
            }
            case APPLY_SHARE_CONNECT_RES:
            {
                // 控制方收到被控制方申请共享连接的回复
                OutData data;
                data.type = APPLY_SHARE_CONNECT_RES;
                _outgo_chan << data;
                self->handleRemoteShareConnectReply(json_obj);
                break;
            }
            case SHARE_START:
            {
                // 被控制方收到控制方的开始共享
                OutData data;
                data.type = SHARE_START;
                _outgo_chan << data;
                self->handleRemoteShareStart(json_obj);
                break;
            }
            case SHARE_START_RES:
            {
                // 被控制方收到控制方的开始共享
                OutData data;
                data.type = SHARE_START_RES;
                _outgo_chan << data;
                self->handleRemoteShareStartRes(json_obj);
                break;
            }
            case SHARE_STOP:
            {
                // 被控制方收到控制方的共享停止
                OutData data;
                data.type = SHARE_STOP;
                _outgo_chan << data;
                self->handleRemoteShareStop(json_obj);
                break;
            }
            case DISCONNECT_CB:
            {
                // 断开连接
                OutData data;
                data.type = DISCONNECT_CB;
                _outgo_chan << data;
                self->handleRemoteDisConnectCb(json_obj);
                break;
            }
            case DISAPPLY_SHARE_CONNECT:
            {
                // 断开连接
                OutData data;
                data.type = DISAPPLY_SHARE_CONNECT;
                _outgo_chan << data;
                self->handleRemoteDisApplyShareConnect(json_obj);
                break;
            }
            case SEARCH_DEVICE_BY_IP:
            {
                if (self)
                    self->handleRemoteSearchIp(json_obj);
                break;
            }
            case DISCOVER_BY_TCP:
            {
                if (self)
                    self->hanldeRemoteDiscover(json_obj);
                break;
            }
            default:{
                OutData data;
                data.type = UNKOWN;
                _outgo_chan << data;
                break;
            }
            }
        }
    });
}

void HandleRpcService::handleTimeOut()
{
    QWriteLocker lk(&_lock);
    auto keys = _ping_lost_count.keys();
    for (const auto &key : keys) {
        auto count = _ping_lost_count.take(key);
        if (count > 3) {
            // 通知客户端连接超时
            ELOG << "timeout: remote server disconnect: " << key.toStdString();
            fastring msg = co::Json({{"app", key.toStdString()}, {"offline", true}}).str();
            SendIpcService::instance()->preprocessOfflineStatus(key, NOPING_TIMEOUT, msg);
            continue;
        }
        _ping_lost_count.insert(key, ++count);
    }
}

void HandleRpcService::handleStartTimer()
{
    if (_timeOut.isActive())
        return;
    _timeOut.start();
}

