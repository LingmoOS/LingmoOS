// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmanager.h"

#include "common/log.h"
#include "common/commonutils.h"
#include "sessionproto.h"
#include "sessionworker.h"
#include "transferworker.h"
#include "filesizecounter.h"

#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

SessionManager::SessionManager(QObject *parent) : QObject(parent)
{
    _trans_workers.clear();
    // Create session and transfer worker
    _session_worker = std::make_shared<SessionWorker>();
    connect(_session_worker.get(), &SessionWorker::onConnectChanged, this, &SessionManager::notifyConnection, Qt::QueuedConnection);
    connect(_session_worker.get(), &SessionWorker::onTransData, this, &SessionManager::handleTransData, Qt::QueuedConnection);
    connect(_session_worker.get(), &SessionWorker::onTransCount, this, &SessionManager::handleTransCount, Qt::QueuedConnection);
    connect(_session_worker.get(), &SessionWorker::onCancelJob, this, &SessionManager::handleCancelTrans, Qt::QueuedConnection);
    connect(_session_worker.get(), &SessionWorker::onRpcResult, this, &SessionManager::handleRpcResult, Qt::QueuedConnection);

    _file_counter = std::make_shared<FileSizeCounter>(this);
    connect(_file_counter.get(), &FileSizeCounter::onCountFinish, this, &SessionManager::handleFileCounted);
}

SessionManager::~SessionManager()
{
    //FIXME: abort if stop them
    //_session_worker->stop();
    //_trans_worker->stop();
}

void SessionManager::setSessionExtCallback(ExtenMessageHandler cb)
{
    _session_worker->setExtMessageHandler(cb);
}

void SessionManager::updatePin(QString code)
{
    _session_worker->updatePincode(code);
}

void SessionManager::setStorageRoot(const QString &root)
{
    _save_root = QString(root);
}

void SessionManager::updateSaveFolder(const QString &folder)
{
    if (_save_root.isEmpty()) {
        _save_root = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    _save_dir = _save_root + QDir::separator();
    if (!folder.isEmpty()) {
        _save_dir += folder + QDir::separator();
    }
}

void SessionManager::updateLoginStatus(QString &ip, bool logined)
{
    _session_worker->updateLogin(ip, logined);
}

void SessionManager::sessionListen(int port)
{
    bool success = _session_worker->startListen(port);
    if (!success) {
        ELOG << "Fail to start listen: " << port;
    }

//    emit notifyConnection(success, "");
}

bool SessionManager::sessionPing(QString ip, int port)
{
    LOG << "sessionPing: " << ip.toStdString();
    return _session_worker->netTouch(ip, port);
}

int SessionManager::sessionConnect(QString ip, int port, QString password)
{
    LOG << "sessionConnect: " << ip.toStdString();
    if (_session_worker->isClientLogin(ip))
        return 1;
    if (!_session_worker->netTouch(ip, port)) {
        ELOG << "Fail to connect remote:" << ip.toStdString();
        return -1;
    }

    // base64 the pwd
    QByteArray pinHash = password.toUtf8().toBase64();
    std::string pinString(pinHash.constData(), pinHash.size());

    LoginMessage req;
    req.name = deepin_cross::CommonUitls::getFirstIp();
    req.auth = pinString;

    QString jsonMsg = req.as_json().serialize().c_str();
    sendRpcRequest(ip, REQ_LOGIN, jsonMsg);

    return 0;
}

void SessionManager::sessionDisconnect(QString ip)
{
    LOG << "session disconnect:" << ip.toStdString();
    _session_worker->disconnectRemote();
}

std::shared_ptr<TransferWorker> SessionManager::createTransWorker(const QString &jobid)
{
    // Create a new TransferWorker
    auto newWorker = std::make_shared<TransferWorker>(jobid);
    // auto newWorker = QSharedPointer<TransferWorker>::create(this);
    connect(newWorker.get(), &TransferWorker::notifyChanged, this, &SessionManager::notifyTransChanged);
    connect(newWorker.get(), &TransferWorker::onException, this, &SessionManager::handleTransException);

    // Store it in the map with the given jobid
    _trans_workers[jobid] = newWorker;

    return newWorker;
}

void SessionManager::sendFiles(QString &ip, int port, QStringList paths)
{
    std::vector<std::string> name_vector;
    std::string token;

    auto worker = createTransWorker(ip);
    bool success = worker->tryStartSend(paths, port, &name_vector, &token);
    if (!success) {
        ELOG << "Fail to send size: " << paths.size() << " at:" << port;
        return;
    }

    QString localIp = deepin_cross::CommonUitls::getFirstIp().data();
    QString accesstoken = QString::fromStdString(token);
    QString endpoint = QString("%1:%2:%3").arg(localIp).arg(port).arg(accesstoken);
    int64_t total = _file_counter->countFiles(ip, paths);
    bool needCount = total == 0;

    TransDataMessage req;
    req.id = ip.toStdString();
    req.names = name_vector;
    req.endpoint = endpoint.toStdString();
    req.flag = needCount; // many folders
    req.size = total; // unkown size

    QString jsonMsg = req.as_json().serialize().c_str();
    sendRpcRequest(ip, REQ_TRANS_DATAS, jsonMsg);

    if (total > 0) {
        QString oneName = paths.join(";");
        handleTransCount(oneName, total);
    }
}

void SessionManager::recvFiles(QString &ip, int port, QString &token, QStringList names)
{
    auto worker = createTransWorker(ip);
    bool success = worker->tryStartReceive(names, ip, port, token, _save_dir);
    if (!success) {
        ELOG << "Fail to recv name size: " << names.size() << " at:" << ip.toStdString();
    }
}

void SessionManager::cancelSyncFile(const QString &ip, const QString &reason)
{
    DLOG << "cancelSyncFile to: " << ip.toStdString();

    // first: send cancel rpc, target jobid is self ip addr.
    TransCancelMessage req;
    req.id = deepin_cross::CommonUitls::getFirstIp();
    req.name = "all";
    req.reason = reason.toStdString();

    QString jsonMsg = req.as_json().serialize().c_str();
    sendRpcRequest(ip, REQ_TRANS_CANCLE, jsonMsg);

    // then: stop local worker
    handleCancelTrans(ip, reason);
}

void SessionManager::sendRpcRequest(const QString &ip, int type, const QString &reqJson)
{
    proto::OriginMessage request;
    request.mask = type;
    request.json_msg = reqJson.toStdString();
#ifdef QT_DEBUG
    DLOG << "sendRpcRequest " << request;
#endif
    _session_worker->sendAsyncRequest(ip, request);
}

void SessionManager::handleTransData(const QString endpoint, const QStringList nameVector)
{
    QStringList parts = endpoint.split(":");
    if (parts.length() == 3) {
        // 现在ip、port和token中分别包含了拆解后的内容
        recvFiles(parts[0], parts[1].toInt(), parts[2], nameVector);
    } else {
        // 错误处理，确保parts包含了3个元素
        WLOG << "endpoint format should be: ip:port:token";
    }
}

void SessionManager::handleTransCount(const QString names, quint64 size)
{
    // TRANS_COUNT_SIZE = 50
    emit notifyTransChanged(50, names, size);
}

void SessionManager::handleCancelTrans(const QString jobid, const QString reason)
{
    // stop the worker
    auto it = _trans_workers.find(jobid);
    if (it != _trans_workers.end()) {
        it->second->stop();
        // Remove the worker from the map
        _trans_workers.erase(it);
    }

    if (!reason.isEmpty()) {
        // TRANS_EXCEPTION = 49
        emit notifyTransChanged(49, reason, 0);
    } else {
        // TRANS_CANCELED = 48 by user
        emit notifyTransChanged(48, "", 0);
    }
}

void SessionManager::handleFileCounted(const QString ip, const QStringList paths, quint64 totalSize)
{
    if (ip.isEmpty()) {
        WLOG << "empty target address for file counted.";
        return;
    }
    std::vector<std::string> nameVector;
    for (auto path : paths) {
        QFileInfo fileInfo(path);
        std::string name = fileInfo.fileName().toStdString();
        nameVector.push_back(name);
    }

    TransDataMessage req;
    req.id = ip.toStdString();
    req.names = nameVector;
    req.endpoint = "::";
    req.flag = false; // no need count
    req.size = totalSize;

    QString jsonMsg = req.as_json().serialize().c_str();
    sendRpcRequest(ip, INFO_TRANS_COUNT, jsonMsg);

    // notify local
    QString oneName = paths.join(";");
    handleTransCount(oneName, totalSize);
}


void SessionManager::handleRpcResult(int32_t type, const QString &response)
{
#ifdef QT_DEBUG
    DLOG << "RPC Result type=" << type << " response: " << response.toStdString();
#endif
    // notify the result to upper caller
    emit notifyAsyncRpcResult(type, response);
}

void SessionManager::handleTransException(const QString jobid, const QString reason)
{
#ifdef QT_DEBUG
    DLOG << jobid.toStdString() << " transfer occur exception: " << reason.toStdString();
#endif
    cancelSyncFile(jobid, reason);
}
