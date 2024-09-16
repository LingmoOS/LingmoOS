// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatwrapper.h"
#include "compatwrapper_p.h"
#include "common/commonutils.h"

#include "networkutil.h"

#include <QApplication>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QDir>

using namespace cooperation_core;

CompatWrapperPrivate::CompatWrapperPrivate(CompatWrapper *qq)
    : q(qq)
{
    ipcInterface = new CuteIPCInterface();

    ipcTimer = new QTimer(this);
    connect(ipcTimer, &QTimer::timeout, this, &CompatWrapperPrivate::onTimeConnectBackend);
    ipcTimer->setSingleShot(true);
    ipcTimer->start(500);
}

CompatWrapperPrivate::~CompatWrapperPrivate()
{
}

void CompatWrapperPrivate::onTimeConnectBackend()
{
    backendOk = ipcInterface->connectToServer("cooperation-daemon");
    if (backendOk) {
        // bind SIGNAL to SLOT
        ipcInterface->remoteConnect(SIGNAL(dataTransferSignal(int, QString)), this, SLOT(ipcCompatSlot(int, QString)));

        QString who = qApp->applicationName();
        ipcInterface->call("bindSignal", Q_RETURN_ARG(QString, sessionId), Q_ARG(QString, who), Q_ARG(QString, "dataTransferSignal"));

        WLOG << "ping return ID:" << sessionId.toStdString();
    } else {
        WLOG << "can not connect to: cooperation-daemon";
        ipcTimer->start(2000);
    }
}

void CompatWrapperPrivate::ipcCompatSlot(int type, const QString& msg)
{
    // DLOG << "recv IPC:" << type << " " << msg.toStdString();

    picojson::value json_obj;
    std::string err = picojson::parse(json_obj, msg.toStdString());
    if (!err.empty()) {
        WLOG << "Failed to parse JSON data: " << err;
        return;
    }

    switch (type) {
    case ipc::IPC_PING: {

    } break;
    case ipc::FRONT_PEER_CB: {
        //DLOG << "recv FRONT_PEER_CB json:" << json_obj;
    } break;
    case ipc::FRONT_CONNECT_CB: {
        ipc::GenericResult param;
        param.from_json(json_obj);
        QString msg(param.msg.c_str());
        QStringList parts = msg.split(" "); // "ip name"
        QString ip = parts.first(); // get ip address

        DLOG << "FRONT_CONNECT_CB: ip=" << ip.toStdString() << " result=" << param.result;
        emit q->compatConnectResult(param.result, ip);
    } break;
    case ipc::FRONT_TRANS_STATUS_CB: {
        ipc::GenericResult param;
        param.from_json(json_obj);
        QString msg(param.msg.c_str());   // job path

        q->metaObject()->invokeMethod(NetworkUtil::instance(), "compatTransJobStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, param.id),
                                      Q_ARG(int, param.result),
                                      Q_ARG(QString, msg));
    } break;
    case ipc::FRONT_NOTIFY_FILE_STATUS: {
        ipc::FileStatus param;
        param.from_json(json_obj);

        auto name = QString::fromStdString(param.name);
        q->metaObject()->invokeMethod(NetworkUtil::instance(),
                                      "compatFileTransStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, name),
                                      Q_ARG(quint64, param.total),
                                      Q_ARG(quint64, param.current),
                                      Q_ARG(quint64, param.millisec));
    } break;
    case ipc::FRONT_SERVER_ONLINE:
        // backend startup, start discovery. ignore
        // q->metaObject()->invokeMethod(MainController::instance(),
        //                               "start",
        //                               Qt::QueuedConnection);
        break;
    case ipc::FRONT_SEND_STATUS: {
        ipc::SendStatus param;
        param.from_json(json_obj);
        LOG << " FRONT_SEND_STATUS  : " << json_obj;
        //{"curstatus":1,"msg":"{\"app\":\"dde-cooperation\",\"offline\":true}","status":1,"type":2}

        if (param.status < 0) {
            picojson::value obj;
            std::string err = picojson::parse(obj, param.msg);
            if (!err.empty()) {
                WLOG << "Failed to parse status msg: " << err;
                return;
            }
            if (param.type == 999) {
                // LOGIN_INFO
                ipc::SendResult result;
                result.from_json(obj);
                QString ip = QString::fromStdString(result.data);
                emit q->compatConnectResult(param.status, ip);
            }
        }
        break;
    }
    case ipc::MISC_MSG: {
        LOG << " MISC_MSG  : " << json_obj;
        q->metaObject()->invokeMethod(NetworkUtil::instance(),
                                      "handleMiscMessage",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, msg));
    }
    break;
    default:
        break;
    }
}


CompatWrapper::CompatWrapper(QObject *parent)
    : QObject(parent),
    d(new CompatWrapperPrivate(this))
{
}

CompatWrapper::~CompatWrapper()
{
}

CompatWrapper *CompatWrapper::instance()
{
    static CompatWrapper ins;
    return &ins;
}

CuteIPCInterface *CompatWrapper::ipcInterface()
{
    if (!d->ipcInterface)
        d->ipcInterface = new CuteIPCInterface;

    return d->ipcInterface;
}

QString CompatWrapper::session()
{
    return d->sessionId;
}
