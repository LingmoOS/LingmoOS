// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkutil.h"
#include "networkutill_p.h"

#include "protoconstants.h"
#include "manager/sessionmanager.h"
#include "manager/sessionproto.h"
#include "common/log.h"
#include "common/constant.h"
#include "common/commonutils.h"

#include "helper/transferhepler.h"
#include "compatwrapper.h"

#include <QApplication>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QTime>

#include <utils/transferutil.h>

using namespace cooperation_core;
NetworkUtilPrivate::NetworkUtilPrivate(NetworkUtil *qq)
    : q(qq)
{
    ExtenMessageHandler msg_cb([this](int32_t mask, const picojson::value &json_value, std::string *res_msg) -> bool {
        DLOG << "NetworkUtil >> " << mask << " msg_cb, json_msg: " << json_value << std::endl;
        switch (mask) {
        case SESSION_MESSAGE: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
            auto msg = QString::fromStdString(req.nick);
            q->handleMiscMessage(msg);
        }
            return true;
        }
        // unhandle message
        return false;
    });

    sessionManager = new SessionManager(this);
    sessionManager->setSessionExtCallback(msg_cb);

    sessionManager->sessionListen(DATA_SESSION_PORT);

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
    if (result == 2)
        confirmTargetAddress = reason;

    if (result == -1 && confirmTargetAddress == reason)
        TransferHelper::instance()->emitDisconnected();

    if (result == LOGIN_SUCCESS) {
        QString unfinishJson;
        int remainSpace = TransferUtil::getRemainSize();
        bool unfinish = TransferUtil::isUnfinishedJob(unfinishJson, confirmTargetAddress);
#ifdef __linux__
        TransferHelper::instance()->setConnectIP(confirmTargetAddress);
#endif

        if (unfinish)
            TransferHelper::instance()->sendMessage("unfinish_json", unfinishJson);
        TransferHelper::instance()->sendMessage("remaining_space", QString::number(remainSpace));
        emit TransferHelper::instance()->connectSucceed();
        return;
    } else if (result == EX_NETWORK_PINGOUT) {
        emit TransferHelper::instance()->onlineStateChanged(false);
        return;
    }
}

void NetworkUtilPrivate::handleTransChanged(int status, const QString &path, quint64 size)
{
#ifndef __linux__
    Q_UNUSED(status);
    Q_UNUSED(path);
    Q_UNUSED(size);
    return;
#else
    //DLOG << "handleTransChanged" << status << " " << path.toStdString();
    switch (status) {
    case TRANS_CANCELED:
        //cancelTransfer(path.compare("im_sender") == 0);
        break;
    case TRANS_EXCEPTION:
        //TODO: notify show exception UI
        break;
    case TRANS_COUNT_SIZE:
        // only update the total size while rpc notice
        transferInfo.totalSize = size;
        break;
    case TRANS_WHOLE_START:
        emit TransferHelper::instance()->transferring();
        break;
    case TRANS_WHOLE_FINISH: {
        if (!TransferHelper::instance()->getConnectIP().isEmpty())
            TransferHelper::instance()->setting(path);
    } break;
    case TRANS_INDEX_CHANGE: {
        TransferHelper::instance()->addFinshedFiles(finishfile, 0);
        finishfile = path;
        emit TransferHelper::instance()->transferContent(tr("Transfering"), path, -2, -2);
    } break;
    case TRANS_FILE_CHANGE:
        break;
    case TRANS_FILE_SPEED: {
        transferInfo.transferSize += size;
        transferInfo.maxTimeS += 1;   // 每1秒收到此信息
        updateTransProgress(transferInfo.transferSize, "");
    } break;
    case TRANS_FILE_DONE:
        break;
    }
#endif
}

void NetworkUtilPrivate::updateTransProgress(uint64_t current, const QString &path)
{
    QTime time(0, 0, 0);
    if (transferInfo.totalSize < 1) {
        // the total has not been set.
        emit TransferHelper::instance()->transferContent(tr("Transfering"), "", 0, 0);
        return;
    }

    // 计算整体进度和预估剩余时间
    double value = static_cast<double>(current) / transferInfo.totalSize;
    int progressValue = static_cast<int>(value * 100);

    int remain_time;
    if (progressValue <= 0) {
        return;
    } else if (progressValue >= 100) {
        progressValue = 100;
        remain_time = 0;
    } else {
        remain_time = (transferInfo.maxTimeS * 100 / progressValue - transferInfo.maxTimeS);
    }
    time = time.addSecs(remain_time);

    // DLOG << "path-----------" << path.toStdString();
    // DLOG << "progressbar: " << progressValue << " remain_time=" << remain_time;
    // DLOG << "totalSize: " << transferInfo.totalSize << " transferSize=" << current;

    emit TransferHelper::instance()->transferContent(tr("Transfering"), path, progressValue, remain_time);
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

            // q->doNextCombiRequest(ip);
            // notify login success
            emit TransferHelper::instance()->connectSucceed();
        } else {
            // try connect with old protocol
            q->compatLogin();
        }
    } break;
    case SESSION_MESSAGE:
        break;
    default:
        LOG << "unkown rpc callback type: " << type << " response:" << response.toStdString();
        break;
    }
}

NetworkUtil::NetworkUtil(QObject *parent)
    : QObject(parent),
      d(new NetworkUtilPrivate(this))
{
    updateStorageConfig();

    auto wrapper = CompatWrapper::instance();
    connect(wrapper, &CompatWrapper::compatConnectResult, this, &NetworkUtil::handleCompatConnectResult, Qt::QueuedConnection);
}

NetworkUtil::~NetworkUtil()
{
}

NetworkUtil *NetworkUtil::instance()
{
    static NetworkUtil ins;
    return &ins;
}

void NetworkUtil::updateStorageConfig()
{
    auto stordir = TransferUtil::DownLoadDir(false);
    d->sessionManager->updateSaveFolder(stordir);

    // compat old protocol
    {
        auto appName = qAppName();
        auto fullstorpath = TransferUtil::DownLoadDir(true);
        //update the storage dir for old protocol
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("saveAppConfig", Q_ARG(QString, appName), Q_ARG(QString, "storagedir"), Q_ARG(QString, fullstorpath));
    }
}

void NetworkUtil::updatePassword(const QString &code)
{
    d->sessionManager->updatePin(code);

    //update the pincode for old protocol
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("setAuthPassword", Q_ARG(QString, code));
}

bool NetworkUtil::doConnect(const QString &ip, const QString &password)
{
    _loginCombi.first = ip;
    _loginCombi.second = password;

    int logind = d->sessionManager->sessionConnect(ip, DATA_SESSION_PORT, password);
    if (logind > 0) {
        d->confirmTargetAddress = ip;
        return true;
    } else if (logind < 0){
        DLOG << "try connect FAILED, try compat!";
        compatLogin();
    }
    return false;
}

void NetworkUtil::disConnect()
{
    if (!d->confirmTargetAddress.isEmpty()) {
        d->sessionManager->sessionDisconnect(d->confirmTargetAddress);
    } else {
        auto appName = qAppName();
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doDisconnectCallback", Q_ARG(QString, appName));
    }
}

bool NetworkUtil::sendMessage(const QString &message)
{
    if (!d->confirmTargetAddress.isEmpty()) {
        ApplyMessage msg;
        msg.nick = message.toStdString();
        QString jsonMsg = msg.as_json().serialize().c_str();
        d->sessionManager->sendRpcRequest(d->confirmTargetAddress, SESSION_MESSAGE, jsonMsg);
    } else {
        auto msg = message;
        if (msg.contains("add_result")) {
            msg.replace("/", " "); // true或false前后的/
        }

        auto appName = qAppName();
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("sendMiscMessage", Q_ARG(QString, appName), Q_ARG(QString, msg));
    }

    return true;
}

void NetworkUtil::cancelTrans()
{
    if (!d->confirmTargetAddress.isEmpty()) {
        d->sessionManager->cancelSyncFile(d->confirmTargetAddress);
    } else {
        auto appName = qAppName();
        auto jobid = appName.length();
        // TRANS_CANCEL 1008; data transfer jobid: name's length
        bool res =  false;
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doOperateJob", Q_RETURN_ARG(bool, res), Q_ARG(int, 1008), Q_ARG(int, jobid), Q_ARG(QString, appName));
    }
}

void NetworkUtil::doSendFiles(const QStringList &fileList)
{
    if (!d->confirmTargetAddress.isEmpty()) {
        int ranport = deepin_cross::CommonUitls::getAvailablePort();
        d->sessionManager->sendFiles(d->confirmTargetAddress, ranport, fileList);
    } else {
        auto appName = qAppName();
        auto jobid = appName.length();
        auto session = CompatWrapper::instance()->session();
        // auto deviceName = CooperationUtil::deviceInfo().value(AppSettings::DeviceNameKey).toString();
        // QString saveDir = (deviceName + "(%1)").arg(CooperationUtil::localIPAddress());
        // try again with old protocol by daemon
        auto ipc = CompatWrapper::instance()->ipcInterface();
        ipc->call("doTransferFile", Q_ARG(QString, session), Q_ARG(QString, appName),
                  Q_ARG(int, jobid), Q_ARG(QStringList, fileList), Q_ARG(bool, true),
                  Q_ARG(QString, ""));
    }
}

void NetworkUtil::compatLogin()
{
    auto ip = _loginCombi.first;
    auto pwd = _loginCombi.second;

    auto appName = qAppName();
    // try connect with old protocol by daemon
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("doTryConnect", Q_ARG(QString, appName), Q_ARG(QString, appName),
              Q_ARG(QString, ip), Q_ARG(QString, pwd));
}

void NetworkUtil::handleMiscMessage(const QString &msg)
{
    metaObject()->invokeMethod(TransferHelper::instance(),
                               "handleMessage",
                               Qt::QueuedConnection,
                               Q_ARG(QString, msg));
}


// ----------compat the old protocol-----------

void NetworkUtil::handleCompatConnectResult(int result, const QString &ip)
{
    Q_UNUSED(ip);

    if (result > 0) {
        // login success, notify
        QString unfinishJson;
        int remainSpace = TransferUtil::getRemainSize();
        bool unfinish = TransferUtil::isUnfinishedJob(unfinishJson, ip);
#ifdef __linux__
        TransferHelper::instance()->setConnectIP(ip);
#endif

        if (unfinish)
            TransferHelper::instance()->sendMessage("unfinish_json", unfinishJson);
        TransferHelper::instance()->sendMessage("remaining_space", QString::number(remainSpace));
        emit TransferHelper::instance()->connectSucceed();
    } else {
        TransferHelper::instance()->emitDisconnected();
    }
}

void NetworkUtil::compatTransJobStatusChanged(int id, int result, const QString &msg)
{
    LOG << "id: " << id << " result: " << result << " msg: " << msg.toStdString();
    switch (result) {
    case JOB_TRANS_DOING:
        emit TransferHelper::instance()->transferring();
        break;
    case JOB_TRANS_FINISHED: {
#ifndef __linux__
        return;
#else
        if (!TransferHelper::instance()->getConnectIP().isEmpty())
            TransferHelper::instance()->setting(msg);
#endif
    } break;
    case JOB_TRANS_FAILED:
    case JOB_TRANS_CANCELED:
        emit TransferHelper::instance()->interruption();
        TransferHelper::instance()->emitDisconnected();
        break;
    default:
        break;
    }
}

void NetworkUtil::compatFileTransStatusChanged(const QString &path, quint64 total, quint64 current, quint64 millisec)
{
    d->transferInfo.totalSize = total;
    d->transferInfo.transferSize = current;
    d->transferInfo.maxTimeS = millisec / 1000;

    if (!d->transferingFile.startsWith(path)) {
        // the select transfer index changed
#ifdef __linux__
        TransferHelper::instance()->addFinshedFiles(d->finishfile, 0);
        d->finishfile = d->transferingFile;
        emit TransferHelper::instance()->transferContent(tr("Transfering"), path, -2, -2);
#endif
        d->transferingFile = path;
    }

    // 计算整体进度和预估剩余时间
    double value = static_cast<double>(d->transferInfo.transferSize) / d->transferInfo.totalSize;
    int progressValue = static_cast<int>(value * 100);
    QTime time(0, 0, 0);
    int remain_time;
    if (progressValue <= 0) {
        return;
    } else if (progressValue >= 100) {
        progressValue = 100;
        remain_time = 0;
    } else {
        remain_time = (d->transferInfo.maxTimeS * 100 / progressValue - d->transferInfo.maxTimeS);
    }
    time = time.addSecs(remain_time);

    // LOG << "path-----------" << path.toStdString();
    // LOG << "progressbar: " << progressValue << " remain_time=" << remain_time;
    // LOG << "totalSize: " << d->transferInfo.totalSize << " transferSize=" << d->transferInfo.transferSize;

    emit TransferHelper::instance()->transferContent(tr("Transfering"), path, progressValue, remain_time);
}

void NetworkUtil::stop()
{
    auto ipc = CompatWrapper::instance()->ipcInterface();
    ipc->call("appExit");
}
