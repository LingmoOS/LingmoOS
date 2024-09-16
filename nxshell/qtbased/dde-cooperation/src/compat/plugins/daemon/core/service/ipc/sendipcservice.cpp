// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sendipcservice.h"
#include "service/comshare.h"
#include "session.h"
#include "service/discoveryjob.h"
#include "service/share/sharecooperationservicemanager.h"
#include "service/rpc/sendrpcservice.h"
#include "common/constant.h"
#include "ipc/proto/comstruct.h"
#include "ipc/bridge.h"
#include "service/jobmanager.h"

#include <QCoreApplication>
#include <QThread>

SendIpcService::SendIpcService(QObject *parent)
    : QObject(parent)
{
    initConnect();

    _ping.setInterval(1000);
    _ping.start();
}

SendIpcService::~SendIpcService()
{
    handleAboutToQuit();
}

SendIpcService *SendIpcService::instance()
{
    static SendIpcService service;
    return &service;
}

void SendIpcService::handleSaveSession(const QString appName, const QString session, const QString signalName)
{
    QSharedPointer<Session> s(new Session(appName, session, signalName));
    _sessions.remove(appName);
    _sessions.insert(appName, s);
}

// void SendIpcService::handleConnectClosed(const quint16 port)
// {
//     for (auto s = _sessions.begin(); s != _sessions.end();) {
//         if (s.value()->port() == port && !s.value()->alive()) {
//             s = _sessions.erase(s);
//         } else {
//             s++;
//         }
//     }
// }

void SendIpcService::handleRemoveSessionByAppName(const QString appName)
{
    _sessions.remove(appName);
}

void SendIpcService::handleRemoveSessionBySessionID(const QString sessionID)
{
    for (auto s = _sessions.begin(); s != _sessions.end();) {
        if (s.value()->getSession() == sessionID) {
            s = _sessions.erase(s);
        } else {
            s++;
        }
    }
}

void SendIpcService::handleSendToClient(const QString appName, int type, const QString msg)
{
    auto s = _sessions.value(appName);
    if (s.isNull()) {
        if (appName == "daemon-cooperation" || appName == "") {
            // replace as dde-cooperation
            s = _sessions.value("dde-cooperation");
        }
    }
    if (s.isNull()) {
        ELOG << "no session to send, appname =" << appName.toStdString() << ",  msg = " << msg.toStdString();
        return;
    }

    QString sigName = s->signal();
    if (sigName.isEmpty())
        return;

    emit sessionSignal(sigName, type, msg);
}

void SendIpcService::handleSendToAllClient(int type, const QString msg)
{
    for (const auto &appName : _sessions.keys()) {
        handleSendToClient(appName, type, msg);
    }
}

void SendIpcService::handleAboutToQuit()
{
    _ping.stop();
    _cacheTimer.stop();
    _offline_status.clear();
    _sessions.clear();
}

void SendIpcService::handleAddJob(const QString appName, const int jobId)
{
    auto s = _sessions.value(appName);
    if (s.isNull()) {
        ELOG << "no session to add job, appname = " << appName.toStdString() << "  ,  jobID = " << jobId;
        return;
    }
    s->addJob(jobId);
}

void SendIpcService::handleRemoveJob(const QString appName, const int jobId)
{
    auto s = _sessions.value(appName);
    if (s.isNull()) {
        ELOG << "no session to remove job, appname = " << appName.toStdString() << "  ,  jobID = " << jobId;
        return;
    }
    s->removeJob(jobId);
}

void SendIpcService::preprocessOfflineStatus(const QString appName, int32 type, const fastring msg)
{
    //缓存一下要通知的状态，如果3秒未更新，才发送通知
    SendStatus st;
    st.type = type;
    st.status = REMOTE_CLIENT_OFFLINE;
    st.curstatus = Comshare::instance()->currentStatus();
    st.msg = msg;
    _offline_status.remove(appName);
    _offline_status.insert(appName, st);

    _cacheTimer.setInterval(3000);
    connect(&_cacheTimer, &QTimer::timeout, [this, appName]() {
        auto names = _offline_status.keys();
        for (const auto &name : names) {
            auto st = _offline_status.take(name);

            // notifySendStatus
            QString jsonMsg = st.as_json().str().c_str();
            SendIpcService::instance()->handleSendToClient(name, FRONT_SEND_STATUS, jsonMsg);
        }
    });

    emit startOfflineTimer();
}

void SendIpcService::cancelOfflineStatus(const QString appName)
{
    //取消缓存的通知
    _offline_status.remove(appName);
    if (_offline_status.count() == 0 || appName.compare("all") == 0) {
        //DLOG << "cancel offline: " << appName.toStdString();
        emit stopOfflineTimer();
    }
}

void SendIpcService::handleStartOfflineTimer()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    if (!_cacheTimer.isActive())
        _cacheTimer.start();
}

void SendIpcService::handleStopOfflineTimer()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    _cacheTimer.stop();
}

void SendIpcService::initConnect()
{
    connect(this, &SendIpcService::startOfflineTimer, this, &SendIpcService::handleStartOfflineTimer, Qt::QueuedConnection);
    connect(this, &SendIpcService::stopOfflineTimer, this, &SendIpcService::handleStopOfflineTimer, Qt::QueuedConnection);
}


void SendIpcService::handleNodeChanged(bool found, QString info)
{
    // notify to all frontend sessions
    for (auto i = _sessions.begin(); i != _sessions.end();) {
        QSharedPointer<Session> s = *i;
        if (s->alive()) {
            fastring nodeinfo(info.toStdString());
            if (!found && s->getName() == "dde-cooperation") {
                WLOG << "The node losted from discovery.";
                // FIXME: The discovery lost mutli-UDP on some WIFI(unstable)
                // handleStopShareConnect(info, s);
            }

            // cbPeerInfo {GenericResult}
            co::Json req = {
               { "id", 0 },
               { "result", found ? 1 : 0 },
               { "msg", nodeinfo },
               { "isself", false},
            };

            QString jsonMsg = req.str().c_str();
            handleSendToClient(s->getName(), FRONT_PEER_CB, jsonMsg);

            ++i;
        } else {
            SendRpcService::instance()->removePing(s->getName());
            // the frontend is offline
            i = _sessions.erase(i);

            //remove the frontend app register info
            fastring name = s->getName().toStdString();
            DiscoveryJob::instance()->removeAppbyName(name);
        }
    }
}
