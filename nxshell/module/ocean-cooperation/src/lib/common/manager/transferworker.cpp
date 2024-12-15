// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferworker.h"
#include "sessionmanager.h"
#include "secureconfig.h"

#include "common/log.h"
#include "common/constant.h"

#include <QFile>
#include <QStorageInfo>

#include <atomic>

TransferWorker::TransferWorker(QString id, QObject *parent)
    : QObject(parent)
    , _bindId(id)
{
    // create own asio service
    _asioService = std::make_shared<AsioService>();
    if (!_asioService) {
        ELOG << "carete ASIO for transfer worker ERROR!";
    }
    _asioService->Start();

    QObject::connect(this, &TransferWorker::speedTimerTick, this, &TransferWorker::handleTimerTick, Qt::QueuedConnection);
    QObject::connect(&_speedTimer, &QTimer::timeout, this, &TransferWorker::doCalculateSpeed, Qt::QueuedConnection);
}



// return true -> cancel
bool TransferWorker::onProgress(uint64_t size)
{
    _status.secsize.fetch_add(size);

    return _canceled;
}

void TransferWorker::onWebChanged(int state, std::string msg, uint64_t size)
{
    if (state < 1) {
        // errors: WEB_DISCONNECTED = -2,WEB_IO_ERROR = -1,WEB_NOT_FOUND = 0
        emit speedTimerTick(true);
        QString reason = QString::fromStdString(msg);
        emit onException(_bindId, reason);
        return;
    }

    switch(state) {
    case WEB_CONNECTED:
        break;
    case WEB_TRANS_START: {
        DLOG << "notify whole web transfer start!";
        emit speedTimerTick();
        emit notifyChanged(TRANS_WHOLE_START);
    }
        break;
    case WEB_TRANS_FINISH: {
        DLOG << "notify whole web transfer finished!";
        sendTranEndNotify();
    }
        break;
    case WEB_INDEX_BEGIN: {
#ifdef QT_DEBUG
        DLOG << "notify web index: " << msg;
#endif
        QString name = QString::fromStdString(msg);
        emit notifyChanged(TRANS_INDEX_CHANGE, name);
    }
        break;
    case WEB_INDEX_END:
        break;
    case WEB_FILE_BEGIN: {
        _status.path = msg;
        _status.total = size;
        if (_everyNotify) {
            DLOG << "notify file begin: " << msg;
            QString path = QString::fromStdString(msg);
            emit notifyChanged(TRANS_FILE_CHANGE, path, size);
        }
    }
        break;
    case WEB_FILE_END: {
        _status.path = msg;
        _status.total = size;
        if (_everyNotify) {
            DLOG << "notify file end: " << msg;
            QString path = QString::fromStdString(msg);
            emit notifyChanged(TRANS_FILE_DONE, path, size);
        }
    }
        break;
    }
}

void TransferWorker::stop()
{
    _canceled = true;
    emit speedTimerTick(true); // stop the speed timer

    if (_file_server) {
        _file_server->clearBind();
        _file_server->stop();
    }

    if (_file_client) {
        _file_client->stop();
    }
}

bool TransferWorker::tryStartSend(QStringList paths, int port, std::vector<std::string> *nameVector, std::string *token)
{
    _singleFile = false; //reset for send files
    _recvPath = "";

    // first try run web, or prompt error
    if (!startWeb(port)) {
        ELOG << "try to start web sever failed!!!";
        return false;
    }

    picojson::array jsonArray;
    _file_server->clearBind();
    for (auto path : paths) {
        QFileInfo fileInfo(path);
        std::string name = fileInfo.fileName().toStdString();
        nameVector->push_back(name);
        try {
            _file_server->webBind(name, path.toStdString());
        } catch (...) {
            WLOG << "web bind (" << name << ") throw exception.";
        }

        jsonArray.push_back(picojson::value(name));

        if (paths.size() == 1 && fileInfo.isFile()) {
            _singleFile = true;
        }

#ifdef QT_DEBUG
        DLOG << "web bind (" << name << ") on dir: " << path.toStdString();
#endif
    }

    // 将picojson对象转换为字符串
    std::string jsonString = picojson::value(jsonArray).serialize();
    *token = _file_server->genToken(jsonString);

    _canceled = false;
    return true;
}

bool TransferWorker::tryStartReceive(QStringList names, QString &ip, int port, QString &token, QString &dirname)
{
    _singleFile = false; //reset for send files
    // update receive path, while will be notify after whole finish.
    _recvPath = QString(dirname);

    if (!startGet(ip.toStdString(), port)) {
        ELOG << "try to create http Geter failed!!!";
        return false;
    }

    std::string accessToken = token.toStdString();
    std::string savePath = dirname.toStdString();
    _file_client->setConfig(accessToken, savePath);

    std::vector<std::string> webs = _file_client->parseWeb(accessToken);
#ifdef QT_DEBUG
    for (const auto& web : webs) {
        DLOG << "Web: " << web;
    }
#endif
    _file_client->startFileDownload(webs);

    _canceled = false;
    return true;
}

bool TransferWorker::isSyncing()
{
    return !_canceled;
}

void TransferWorker::setEveryFileNotify(bool every)
{
    _everyNotify = every;
}

bool TransferWorker::isServe()
{
    return _recvPath.isEmpty();
}

void TransferWorker::handleTimerTick(bool stop)
{
    if (stop) {
        _speedTimer.stop();
    } else {
        _speedTimer.start(1000);
    }
}

void TransferWorker::doCalculateSpeed()
{

    int64_t bytesize = _status.secsize.load();
    _status.secsize.store(0); // reset every second

//    if (_noDataCount > 10) {
//        DLOG << "10s no data transfer, whole finished!";
//        // 10 seconds did not receive any data.
//        sendTranEndNotify();
//        return;
//    }
    if (bytesize > 0) {
        _noDataCount = 0;
    } else {
        _noDataCount++;
    }

    // double speed = (static_cast<double>(bytesize)) / (1024 * 1024); // 计算下载速度，单位为兆字节/秒
    // QString formattedSpeed = QString::number(speed, 'f', 2); // 格式化速度为保留两位小数的字符串
    // DLOG << "Transfer speed: " << formattedSpeed.toStdString() << " M/s";

    QString path = QString::fromStdString(_status.path);
    emit notifyChanged(TRANS_FILE_SPEED, path, bytesize);
}

void TransferWorker::sendTranEndNotify()
{
    emit speedTimerTick(true);
    emit notifyChanged(TRANS_WHOLE_FINISH, _recvPath);
}


bool TransferWorker::startWeb(int port)
{
    // Create a new file http server
    if (!_file_server) {
        auto context = SecureConfig::serverContext();

        _file_server = std::make_shared<FileServer>(_asioService, context, port);

        auto self(this->shared_from_this());
        _file_server->setCallback(self);
    }

    return _file_server->start();
}

bool TransferWorker::startGet(const std::string &address, int port)
{
    // Create a new file http client
    if (!_file_client) {
        auto context = SecureConfig::clientContext();

        _file_client = std::make_shared<FileClient>(_asioService, context, address, port);

        auto self(this->shared_from_this());
        _file_client->setCallback(self);
    }

    return true;
}
