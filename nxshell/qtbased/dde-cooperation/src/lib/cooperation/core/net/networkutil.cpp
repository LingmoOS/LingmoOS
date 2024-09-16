// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkutil.h"
#include "networkutill_p.h"

#include "manager/sessionmanager.h"
#include "manager/sessionproto.h"
#include "common/commonutils.h"

#include "cooconstrants.h"
#include "discover/discovercontroller.h"
#include "helper/transferhelper.h"
#include "helper/sharehelper.h"
#include "utils/cooperationutil.h"

#include "compatwrapper.h"

#include <QJsonDocument>
#include <QDebug>
#include <QDir>

using namespace cooperation_core;
NetworkUtilPrivate::NetworkUtilPrivate(NetworkUtil *qq)
    : q(qq)
{
    bool onlyTransfer = qApp->property("onlyTransfer").toBool();
    LOG << "This is only transfer?" << onlyTransfer;

    sessionManager = new SessionManager(this);
    if (onlyTransfer) {
        return;
    }
    servePort = COO_SESSION_PORT;

    ExtenMessageHandler msg_cb([this](int32_t mask, const picojson::value &json_value, std::string *res_msg) -> bool {
#ifdef QT_DEBUG
        DLOG << "NetworkUtil >> " << mask << " msg_cb, json_msg: " << json_value << std::endl;
#endif
        switch (mask) {
        case APPLY_INFO: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_DONE;
            WLOG << "msg_cb: " << json_value;
            // response my device info.
            res.nick = q->deviceInfoStr().toStdString();
            *res_msg = res.as_json().serialize();

            // update this device info to discovery list
            q->metaObject()->invokeMethod(DiscoverController::instance(),
                                          "addSearchDeivce",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, QString(req.nick.c_str())));
        }
            return true;
        case APPLY_TRANS: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_WAIT;
            *res_msg = res.as_json().serialize();
            confirmTargetAddress = QString::fromStdString(req.host);

            q->metaObject()->invokeMethod(TransferHelper::instance(),
                                          "notifyTransferRequest",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, QString(req.nick.c_str())),
                                          Q_ARG(QString, QString(req.host.c_str())));
        }
            return true;
        case APPLY_TRANS_RESULT: {
            ApplyMessage req, res;
            req.from_json(json_value);
            bool agree = (req.flag == REPLY_ACCEPT);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
            q->metaObject()->invokeMethod(TransferHelper::instance(),
                                          agree ? "accepted" : "rejected",
                                          Qt::QueuedConnection);
        }
            return true;
        case APPLY_SHARE: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_WAIT;
            QString info = QString::fromStdString(req.host + "," + req.nick + "," + req.fingerprint);
            *res_msg = res.as_json().serialize();
            confirmTargetAddress = QString::fromStdString(req.host);
            q->metaObject()->invokeMethod(ShareHelper::instance(),
                                          "notifyConnectRequest",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, info));
        }
            return true;
        case APPLY_SHARE_RESULT: {
            ApplyMessage req, res;
            req.from_json(json_value);
            bool agree = (req.flag == REPLY_ACCEPT);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
            q->metaObject()->invokeMethod(ShareHelper::instance(),
                                          "handleConnectResult",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, agree ? SHARE_CONNECT_COMFIRM : SHARE_CONNECT_REFUSE),
                                          Q_ARG(QString, QString(req.fingerprint.c_str())));
        }
            return true;
        case APPLY_SHARE_STOP: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
            q->metaObject()->invokeMethod(ShareHelper::instance(),
                                          "handleDisConnectResult",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, QString(req.host.c_str())));
        }
            return true;
        case APPLY_CANCELED: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
            if (req.nick == "share") {
                q->metaObject()->invokeMethod(ShareHelper::instance(),
                                              "handleCancelCooperApply",
                                              Qt::QueuedConnection);
            } else if (req.nick == "transfer") {
                q->metaObject()->invokeMethod(TransferHelper::instance(),
                                              "handleCancelTransferApply",
                                              Qt::QueuedConnection);
            }
        }
            return true;
        }

        // unhandle message
        return false;
    });

    sessionManager->setSessionExtCallback(msg_cb);
    sessionManager->updatePin(COO_HARD_PIN);
    sessionManager->sessionListen(servePort);

    connect(sessionManager, &SessionManager::notifyConnection, this, &NetworkUtilPrivate::handleConnectStatus);
    connect(sessionManager, &SessionManager::notifyTransChanged, this, &NetworkUtilPrivate::handleTransChanged);
    connect(sessionManager, &SessionManager::notifyAsyncRpcResult, this, &NetworkUtilPrivate::handleAsyncRpcResult);
}

NetworkUtilPrivate::~NetworkUtilPrivate()
{
}

void NetworkUtilPrivate::handleConnectStatus(int result, QString reason)
{
    DLOG << " connect status: " << result << " " << reason.toStdString();
    if (result == 113 || result == 110) {
        // host unreachable or timeout
        q->metaObject()->invokeMethod(DiscoverController::instance(),
                                      "addSearchDeivce",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, ""));
        return;
    }
    if (result == EX_NETWORK_PINGOUT) {
        // ping <-> pong timeout, network connection exception
        // 1. update the discovery list.
        q->metaObject()->invokeMethod(DiscoverController::instance(),
                                      "compatRemoveDeivce",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, reason));

        // 2. show UI if there is any doing job
        q->metaObject()->invokeMethod(ShareHelper::instance(),
                                      "onShareExcepted",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, EX_NETWORK_PINGOUT),
                                      Q_ARG(QString, reason));
        // show UI for transfering
        q->metaObject()->invokeMethod(TransferHelper::instance(), "onTransferExcepted",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, EX_NETWORK_PINGOUT),
                                      Q_ARG(QString, reason));
    } else if (result == -2) {
        // error hanppend
        DLOG << "connect error, reason = " << reason.toStdString();
    } else if (result == -1) {
        // disconnected
        q->metaObject()->invokeMethod(TransferHelper::instance(), "onConnectStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, 0),
                                      Q_ARG(QString, reason),
                                      Q_ARG(bool, false));
    } else if (result == 2) {
        // connected
    }
}

void NetworkUtilPrivate::handleTransChanged(int status, const QString &path, quint64 size)
{
    q->metaObject()->invokeMethod(TransferHelper::instance(),
                                  "onTransChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, status),
                                  Q_ARG(QString, path),
                                  Q_ARG(quint64, size));
}

void NetworkUtilPrivate::handleAsyncRpcResult(int32_t type, const QString response)
{
    picojson::value json_value;
    bool success = false;
    if (!response.isEmpty()) {
        std::string err = picojson::parse(json_value, response.toStdString());
        success = err.empty();
    }

    switch(type) {
    case APPLY_LOGIN: {
        DLOG << "Login return: " << response.toStdString();
        if (success) {
            LoginMessage login;
            login.from_json(json_value);

            QString ip = login.name.c_str();
            bool logined = !login.auth.empty();
            sessionManager->updateLoginStatus(ip, logined);
            // ApplyMessage reply;
            // reply.from_json(json_value);
            // next combi request
            q->doNextCombiRequest(ip);
        } else {
            // try connect with old protocol
            q->compatLogin(response);
        }
        break;
    }
    case APPLY_INFO: {
        DLOG << "Apply info return: " << response.toStdString();
        if (success) {
            ApplyMessage reply;
            reply.from_json(json_value);
            // update this device info to discovery list
            q->metaObject()->invokeMethod(DiscoverController::instance(),
                                          "addSearchDeivce",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, QString(reply.nick.c_str())));
        }
        break;
    }
    case APPLY_TRANS: {
        q->metaObject()->invokeMethod(TransferHelper::instance(),
                                      "onConnectStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, success ? 1 : 0),
                                      Q_ARG(QString, confirmTargetAddress),
                                      Q_ARG(bool, true));
        break;
    }
    case APPLY_SHARE: {
        if (!success) {
            q->metaObject()->invokeMethod(ShareHelper::instance(),
                                          "handleConnectResult",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, SHARE_CONNECT_UNABLE));
        }
        break;
    }
    default:
        LOG << "unkown rpc callback type: " << type << " response:" << response.toStdString();
        break;
    }
}

NetworkUtil::NetworkUtil(QObject *parent)
    : QObject(parent),
      d(new NetworkUtilPrivate(this))
{
    auto wrapper = CompatWrapper::instance();
    auto discover = DiscoverController::instance();
    connect(wrapper, &CompatWrapper::compatConnectResult, this, &NetworkUtil::handleCompatConnectResult, Qt::QueuedConnection);
    connect(discover, &DiscoverController::registCompatAppInfo, this, &NetworkUtil::handleCompatRegister, Qt::QueuedConnection);
    connect(discover, &DiscoverController::startDiscoveryDevice, this, &NetworkUtil::handleCompatDiscover, Qt::QueuedConnection);
}

NetworkUtil::~NetworkUtil()
{
}

NetworkUtil *NetworkUtil::instance()
{
    static NetworkUtil ins;
    return &ins;
}

void NetworkUtil::handleCompatConnectResult(int result, const QString &ip)
{
    auto type = _nextCombi.first;
    if (type == APPLY_TRANS) {
        metaObject()->invokeMethod(TransferHelper::instance(),
                                   "onConnectStatusChanged",
                                   Qt::QueuedConnection,
                                   Q_ARG(int, result),
                                   Q_ARG(QString, ip),
                                   Q_ARG(bool, true));
    } else if (type == APPLY_SHARE) {
        if (result <= 0) {
            metaObject()->invokeMethod(ShareHelper::instance(),
                                       "handleConnectResult",
                                       Qt::QueuedConnection,
                                       Q_ARG(int, SHARE_CONNECT_UNABLE));
        }
    } else if (type == APPLY_INFO) {
        if (result <= 0) {
            metaObject()->invokeMethod(DiscoverController::instance(),
                                       "addSearchDeivce",
                                       Qt::QueuedConnection,
                                       Q_ARG(QString, ""));
        }
    }

    if (result > 0) {
        // login success, do next request.
        doNextCombiRequest(ip, true);
    } else {
        //clearup the pending request if failed.
        _nextCombi.first = 0;
        _nextCombi.second = "";
    }
}

void NetworkUtil::handleCompatRegister(bool reg, const QString &infoJson)
{
    auto ipc = CompatWrapper::instance()->ipcInterface();
    if (reg) {
        ipc->call("registerDiscovery", Q_ARG(bool, false), Q_ARG(QString, ipc::CooperRegisterName), Q_ARG(QString, infoJson));
    } else {
        ipc->call("registerDiscovery", Q_ARG(bool, true), Q_ARG(QString, ipc::CooperRegisterName), Q_ARG(QString, ""));
    }
}

void NetworkUtil::handleCompatDiscover()
{
    auto ipc = CompatWrapper::instance()->ipcInterface();
    QString nodesJson;
    ipc->call("getDiscovery", Q_RETURN_ARG(QString, nodesJson));

    // DLOG << "discovery return:" << nodesJson.toStdString();
    if (!nodesJson.isEmpty()) {
        picojson::value json_value;
        std::string err = picojson::parse(json_value, nodesJson.toStdString());
        if (!err.empty()) {
            WLOG << "Incorrect node list format: " << nodesJson.toStdString();
            return;
        }
        ipc::NodeList nodeList;
        nodeList.from_json(json_value);

        // typedef QMap<QString, QString> StringMap;
        StringMap infoMap;
        for (const auto &peerInfo : nodeList.peers) {
            auto ip = QString::fromStdString(peerInfo.os.ipv4);
            auto sharedip = QString::fromStdString(peerInfo.os.share_connect_ip);
            for (const auto &appInfo : peerInfo.apps) {
                if (appInfo.appname != ipc::CooperRegisterName)
                    continue;

                auto info = QString::fromStdString(appInfo.json);
                auto combinedIP = ip + ", " + sharedip;
                infoMap.insert(info, combinedIP);
            }
        }

        if (!infoMap.empty()) {
            // update this device info to discovery list
            metaObject()->invokeMethod(DiscoverController::instance(),
                                       "compatAddDeivces",
                                       Qt::QueuedConnection,
                                       Q_ARG(StringMap, infoMap));
        }
    }
}

void NetworkUtil::updateStorageConfig(const QString &value)
{
    d->sessionManager->setStorageRoot(value);
    d->storageRoot = value;

    //update the storage dir for old protocol
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("saveAppConfig", Q_ARG(QString, ipc::CooperRegisterName), Q_ARG(QString, "storagedir"), Q_ARG(QString, value));
}

void NetworkUtil::setStorageFolder(const QString &folder)
{
    d->storageFolder = folder;
}

QString NetworkUtil::getStorageFolder() const
{
    return d->storageRoot + QDir::separator() + d->storageFolder;
}

QString NetworkUtil::getConfirmTargetAddress() const
{
    return d->confirmTargetAddress;
}

void NetworkUtil::trySearchDevice(const QString &ip)
{
    DLOG << "searching " << ip.toStdString();

    _nextCombi.first = APPLY_INFO;
    _nextCombi.second = ip;
    // session connect and then send rpc request
    int logind = d->sessionManager->sessionConnect(ip, d->servePort, COO_HARD_PIN);
    if (logind < 0) {
        DLOG << "try apply search FAILED, try compat!";
        compatLogin(ip);
    } else if (logind > 0) {
        // has been login, do next.
        doNextCombiRequest(ip);
    }
}

void NetworkUtil::pingTarget(const QString &ip)
{
    // session connect by async, handle status in callback
    d->sessionManager->sessionPing(ip, d->servePort);
}

void NetworkUtil::reqTargetInfo(const QString &ip, bool compat)
{
    if (compat) {
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doAsyncSearch", Q_ARG(QString, ip), Q_ARG(bool, false));
    } else {
        // send info apply, and sync handle
        ApplyMessage msg;
        msg.flag = ASK_QUIET;
        msg.host = ip.toStdString();
        msg.nick = deviceInfoStr().toStdString();
        QString jsonMsg = msg.as_json().serialize().c_str();
        d->sessionManager->sendRpcRequest(ip, APPLY_INFO, jsonMsg);
        // handle callback result in handleAsyncRpcResult
    }
}

void NetworkUtil::compatLogin(const QString &ip)
{
    auto appName = qAppName();
    // try connect with old protocol by daemon
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("doTryConnect", Q_ARG(QString, appName), Q_ARG(QString, ipc::CooperRegisterName),
              Q_ARG(QString, ip), Q_ARG(QString, ""));
}

void NetworkUtil::doNextCombiRequest(const QString &ip, bool compat)
{
    auto type = _nextCombi.first;
    auto comip = _nextCombi.second;
    if (type <= 0)
        return;

    switch(type) {

    case APPLY_INFO: {
        reqTargetInfo(comip, compat);
        break;
    }
    case APPLY_TRANS: {
        sendTransApply(comip, compat);
        break;
    }
    case APPLY_SHARE: {
        sendShareApply(comip, compat);
        break;
    }
    default:
        WLOG << "unkown next combi type: " << type;
        break;
    }
    //reset
    _nextCombi.first = 0;
    _nextCombi.second = "";
}

void NetworkUtil::tryTransApply(const QString &ip)
{
    _nextCombi.first = APPLY_TRANS;
    _nextCombi.second = ip;

    // session connect and then send rpc request
    int logind = d->sessionManager->sessionConnect(ip, d->servePort, COO_HARD_PIN);
    if (logind < 0) {
        DLOG << "try apply trans FAILED, try compat!";
        compatLogin(ip);
    } else if (logind > 0) {
        // has been login, do next.
        doNextCombiRequest(ip);
    }
}

void NetworkUtil::sendTransApply(const QString &ip, bool compat)
{
    auto deviceName = CooperationUtil::deviceInfo().value(AppSettings::DeviceNameKey).toString();
    if (compat) {
        auto appName = qAppName();
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doApplyTransfer", Q_ARG(QString, appName), Q_ARG(QString, ipc::CooperRegisterName), Q_ARG(QString, deviceName));
    } else {
        // update the target address
        d->confirmTargetAddress = ip;

        // send transfer apply, and async handle in RPC recv
        ApplyMessage msg;
        msg.flag = ASK_NEEDCONFIRM;
        msg.nick = deviceName.toStdString();   // user define nice name
        msg.host = CooperationUtil::localIPAddress().toStdString();
        QString jsonMsg = msg.as_json().serialize().c_str();
        d->sessionManager->sendRpcRequest(ip, APPLY_TRANS, jsonMsg);
    }
}

void NetworkUtil::tryShareApply(const QString &ip, const QString &selfprint)
{
    _nextCombi.first = APPLY_SHARE;
    _nextCombi.second = ip;

    _selfFingerPrint = selfprint;

    int logind = d->sessionManager->sessionConnect(ip, d->servePort, COO_HARD_PIN);
    if (logind < 0) {
        DLOG << "try apply share FAILED, try compat!";
        compatLogin(ip);
    } else if (logind > 0) {
        // has been login, do next.
        doNextCombiRequest(ip);
    }
}

void NetworkUtil::sendShareApply(const QString &ip, bool compat)
{
    DeviceInfoPointer selfinfo = DiscoverController::selfInfo();
    if (compat) {
        QStringList dataInfo({ selfinfo->deviceName(),
                              selfinfo->ipAddress() });
        QString data = dataInfo.join(',');
        auto appName = qAppName();
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doApplyShare", Q_ARG(QString, appName), Q_ARG(QString, appName), Q_ARG(QString, ip), Q_ARG(QString, data));
    } else {
        // update the target address
        d->confirmTargetAddress = ip;

               // session connect and then send rpc request
        ApplyMessage msg;
        msg.flag = ASK_NEEDCONFIRM;
        msg.nick = selfinfo->deviceName().toStdString();
        msg.host = CooperationUtil::localIPAddress().toStdString();
        msg.fingerprint = _selfFingerPrint.toStdString(); // send self fingerprint
        QString jsonMsg = msg.as_json().serialize().c_str();
        d->sessionManager->sendRpcRequest(ip, APPLY_SHARE, jsonMsg);
    }
}

void NetworkUtil::sendDisconnectShareEvents(const QString &ip)
{
    DeviceInfoPointer selfinfo = DiscoverController::selfInfo();
    if (ip == d->confirmTargetAddress) {
        // session connect and then send rpc request
        ApplyMessage msg;
        msg.flag = ASK_NEEDCONFIRM;
        msg.nick = selfinfo->deviceName().toStdString();
        msg.host = CooperationUtil::localIPAddress().toStdString();
        QString jsonMsg = msg.as_json().serialize().c_str();
        d->sessionManager->sendRpcRequest(ip, APPLY_SHARE_STOP, jsonMsg);
    } else {
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doDisconnectShare", Q_ARG(QString, qAppName()), Q_ARG(QString, qAppName()), Q_ARG(QString, selfinfo->deviceName()));
    }
}

void NetworkUtil::replyTransRequest(bool agree)
{
    if (!d->confirmTargetAddress.isEmpty()) {
        // send transfer reply, skip result
        ApplyMessage msg;
        msg.flag = agree ? REPLY_ACCEPT : REPLY_REJECT;
        msg.host = CooperationUtil::localIPAddress().toStdString();
        QString jsonMsg = msg.as_json().serialize().c_str();

        // _confirmTargetAddress
        d->sessionManager->sendRpcRequest(d->confirmTargetAddress, APPLY_TRANS_RESULT, jsonMsg);

        d->sessionManager->updateSaveFolder(d->storageFolder);
    } else {
        auto deviceName = CooperationUtil::deviceInfo().value(AppSettings::DeviceNameKey).toString();
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doReplyTransfer", Q_ARG(QString, ipc::CooperRegisterName), Q_ARG(QString, ipc::CooperRegisterName),
                  Q_ARG(QString, deviceName), Q_ARG(bool, agree));
    }
}

void NetworkUtil::replyShareRequest(bool agree, const QString &selfprint)
{
    if (!d->confirmTargetAddress.isEmpty()) {
        // send transfer reply, skip result
        ApplyMessage msg;
        msg.flag = agree ? REPLY_ACCEPT : REPLY_REJECT;
        msg.host = CooperationUtil::localIPAddress().toStdString();
        msg.fingerprint = selfprint.toStdString(); // send self fingerprint
        QString jsonMsg = msg.as_json().serialize().c_str();

        // _confirmTargetAddress
        d->sessionManager->sendRpcRequest(d->confirmTargetAddress, APPLY_SHARE_RESULT, jsonMsg);
    } else {
        int reply = agree ? SHARE_CONNECT_COMFIRM : SHARE_CONNECT_REFUSE;
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doReplyShare", Q_ARG(QString, qAppName()), Q_ARG(QString, qAppName()), Q_ARG(int, reply));
    }
}

void NetworkUtil::cancelApply(const QString &type)
{
    if (!d->confirmTargetAddress.isEmpty()) {
        ApplyMessage msg;
        msg.nick = type.toStdString();
        QString jsonMsg = msg.as_json().serialize().c_str();
        d->sessionManager->sendRpcRequest(d->confirmTargetAddress, APPLY_CANCELED, jsonMsg);
    } else {
        // FIXME: cancel trans apply
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doCancelShareApply", Q_ARG(QString, qAppName()));
    }
}

void NetworkUtil::cancelTrans()
{
    if (!d->confirmTargetAddress.isEmpty()) {
        d->sessionManager->cancelSyncFile(d->confirmTargetAddress);
    } else {
        // TRANS_CANCEL 1008; coopertion jobid: 1000
        bool res =  false;
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doOperateJob", Q_RETURN_ARG(bool, res), Q_ARG(int, 1008), Q_ARG(int, 1000), Q_ARG(QString, qAppName()));
    }
}

void NetworkUtil::doSendFiles(const QStringList &fileList)
{
    if (!d->confirmTargetAddress.isEmpty()) {
        int ranport = deepin_cross::CommonUitls::getAvailablePort();
        d->sessionManager->sendFiles(d->confirmTargetAddress, ranport, fileList);
    } else {
        auto session = CompatWrapper::instance()->session();
        auto deviceName = CooperationUtil::deviceInfo().value(AppSettings::DeviceNameKey).toString();
        QString saveDir = (deviceName + "(%1)").arg(CooperationUtil::localIPAddress());
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doTransferFile", Q_ARG(QString, session), Q_ARG(QString, ipc::CooperRegisterName),
                  Q_ARG(int, 1000), Q_ARG(QStringList, fileList), Q_ARG(bool, true),
                  Q_ARG(QString, saveDir));
    }
}

QString NetworkUtil::deviceInfoStr()
{
    auto infomap = CooperationUtil::deviceInfo();

    // 将QVariantMap转换为QJsonDocument转换为QString
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(infomap);
    QString jsonString = jsonDocument.toJson(QJsonDocument::Compact);

    return jsonString;
}


void NetworkUtil::compatSendStartShare(const QString &screenName)
{
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("doStartShare", Q_ARG(QString, qAppName()), Q_ARG(QString, screenName));
}

void NetworkUtil::stop()
{
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("appExit");
}
