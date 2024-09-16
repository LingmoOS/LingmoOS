// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatwrapper.h"
#include "compatwrapper_p.h"
#include "common/commonutils.h"
#include "cooconstrants.h"

#include "discover/discovercontroller.h"
#include "helper/transferhelper.h"
#include "helper/sharehelper.h"

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
        ipcInterface->remoteConnect(SIGNAL(cooperationSignal(int, QString)), this, SLOT(ipcCompatSlot(int, QString)));

        QString who = qApp->applicationName();
        ipcInterface->call("bindSignal", Q_RETURN_ARG(QString, sessionId), Q_ARG(QString, who), Q_ARG(QString, "cooperationSignal"));

        WLOG << "ping return ID:" << sessionId.toStdString();
    } else {
        WLOG << "can not connect to: cooperation-daemon";
        ipcTimer->start(2000);
    }
}

void CompatWrapperPrivate::ipcCompatSlot(int type, const QString& msg)
{
    //DLOG << "recv IPC:" << type << " " << msg.toStdString();

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
        // WLOG << "recv IPC json:" << json_obj;

        ipc::GenericResult param;
        param.from_json(json_obj);
        bool find = param.result > 0;

        picojson::value obj;
        std::string err = picojson::parse(obj, param.msg);
        if (!err.empty()) {
            WLOG << "Failed to parse peer data: " << err;
            return;
        }

        ipc::NodeInfo nodeInfo;
        nodeInfo.from_json(obj);

        auto ip = QString::fromStdString(nodeInfo.os.ipv4);
        auto sharedip = QString::fromStdString(nodeInfo.os.share_connect_ip);

        // typedef QMap<QString, QString> StringMap;
        StringMap infoMap;
        for (const auto &appInfo : nodeInfo.apps) {
            // DLOG << find << " Append peer : " << appInfo.appname << " " << appInfo.json;
            // 非跨端应用无需处理
            if (appInfo.appname != ipc::CooperRegisterName)
                continue;

            auto info = QString::fromStdString(appInfo.json);
            auto combinedIP = ip + ", " + sharedip;
            infoMap.insert(info, combinedIP);
        }

        if (infoMap.empty()) {
            if (!find) {
                q->metaObject()->invokeMethod(DiscoverController::instance(),
                                              "compatRemoveDeivce",
                                              Qt::QueuedConnection,
                                              Q_ARG(QString, ip));
            }
        } else {
            // update this device info to discovery list
            q->metaObject()->invokeMethod(DiscoverController::instance(),
                                          "compatAddDeivces",
                                          Qt::QueuedConnection,
                                          Q_ARG(StringMap, infoMap));
        }
    } break;
    case ipc::FRONT_CONNECT_CB: {
        ipc::GenericResult param;
        param.from_json(json_obj);
        QString msg(param.msg.c_str());
        QStringList parts = msg.split(" "); // "ip name"
        QString ip = parts.first(); // get ip address

        DLOG << "FRONT_CONNECT_CB: ip=" << ip.toStdString();
        emit q->compatConnectResult(param.result, ip);
    } break;
    case ipc::FRONT_TRANS_STATUS_CB: {
        ipc::GenericResult param;
        param.from_json(json_obj);
        QString msg(param.msg.c_str());   // job path

        q->metaObject()->invokeMethod(TransferHelper::instance(), "compatTransJobStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, param.id),
                                      Q_ARG(int, param.result),
                                      Q_ARG(QString, msg));
    } break;
    case ipc::FRONT_NOTIFY_FILE_STATUS: {
        ipc::FileStatus param;
        param.from_json(json_obj);

        q->metaObject()->invokeMethod(TransferHelper::instance(),
                                      "compatFileTransStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(quint64, param.total),
                                      Q_ARG(quint64, param.current),
                                      Q_ARG(quint64, param.millisec));
    } break;
    case ipc::FRONT_APPLY_TRANS_FILE: {
        ipc::ApplyTransFiles transferInfo;
        transferInfo.from_json(json_obj);
        LOG << "apply transfer info: " << json_obj;

        if (transferInfo.type == ipc::ApplyTransType::APPLY_TRANS_APPLY) {
            // show confirm UI
            QString ip = QString::fromStdString(transferInfo.selfIp);
            QString host = QString::fromStdString(transferInfo.machineName);
            q->metaObject()->invokeMethod(TransferHelper::instance(),
                                          "notifyTransferRequest",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, host),
                                          Q_ARG(QString, ip));
        } else {
            //ipc::ApplyTransType::APPLY_TRANS_REFUSED  false
            bool agree = (transferInfo.type == ipc::ApplyTransType::APPLY_TRANS_CONFIRM);
            q->metaObject()->invokeMethod(TransferHelper::instance(),
                                          agree ? "accepted" : "rejected",
                                          Qt::QueuedConnection);
        }
    } break;
    case ipc::FRONT_SERVER_ONLINE:
        // backend startup, start discovery. ignore
        // q->metaObject()->invokeMethod(MainController::instance(),
        //                               "start",
        //                               Qt::QueuedConnection);
        break;
    case ipc::FRONT_SHARE_APPLY_CONNECT: {
        ipc::ShareConnectApply conApply;
        conApply.from_json(json_obj);
        // "data":"zerowf,10.8.11.98" -> "data":"10.8.11.98,zerowf"
        QStringList parts = QString::fromStdString(conApply.data).split(",");
        QString info = parts.at(1) + "," + parts.at(0);

        LOG << "share apply info: " << json_obj;
        q->metaObject()->invokeMethod(ShareHelper::instance(),
                                      "notifyConnectRequest",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, info));
    } break;
    case ipc::FRONT_SHARE_APPLY_CONNECT_REPLY: {
        ipc::ShareConnectReply conReply;
        conReply.from_json(json_obj);

        LOG << "share apply connect info: " << json_obj;
        q->metaObject()->invokeMethod(ShareHelper::instance(),
                                      "handleConnectResult",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, conReply.reply),
                                      Q_ARG(QString, "")); // non fingerprint
    } break;
    case ipc::FRONT_SHARE_DISCONNECT: {
        ipc::ShareDisConnect disCon;
        disCon.from_json(json_obj);

        LOG << "share disconnect info: " << json_obj;
        q->metaObject()->invokeMethod(ShareHelper::instance(),
                                      "handleDisConnectResult",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, QString(disCon.msg.c_str())));
    } break;
    case ipc::FRONT_SHARE_DISAPPLY_CONNECT: {
        ipc::ShareConnectDisApply param;
        param.from_json(json_obj);
        LOG << "share cancel apply : " << json_obj;
        q->metaObject()->invokeMethod(ShareHelper::instance(),
                                      "handleCancelCooperApply",
                                      Qt::QueuedConnection);
    } break;
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
    case ipc::FRONT_SEARCH_IP_DEVICE_RESULT: {
        ipc::SearchDeviceResult param;
        param.from_json(json_obj);
        WLOG << "SearchDeviceResult : " << json_obj;
        QString info = param.result ? msg : "";
        // update this device info to discovery list
        q->metaObject()->invokeMethod(DiscoverController::instance(),
                                      "addSearchDeivce",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, info));
    } break;
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
