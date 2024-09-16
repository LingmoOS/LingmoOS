// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sync_client.h"

#include <QDebug>
#include <QtDBus>
#include <QVariantMap>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QProcess>
#include <QFile>
#include <QUrl>
#include <QDesktopServices>
#include <QCoreApplication>

#include "ipc/const.h"
#include "ipc/deepinid_interface.h"

namespace ddc
{
Q_LOGGING_CATEGORY(deepinid_client, "ui.sync_client")

class SyncClientPrivate
{
public:
    explicit SyncClientPrivate(SyncClient *parent)
        : q_ptr(parent)
    {

        daemonIf = new DeepinIDInterface(Const::SyncDaemonService,
                                         Const::SyncDaemonPath,
                                         QDBusConnection::sessionBus());
    }

    DeepinIDInterface *daemonIf;
    QVariantMap session;

    SyncClient *q_ptr;
    Q_DECLARE_PUBLIC(SyncClient)
};

SyncClient::SyncClient(QObject *parent)
    :
    QObject(parent), dd_ptr(new SyncClientPrivate(this))
{

}

SyncClient::~SyncClient() = default;

QString SyncClient::machineID() const
{
    Q_D(
    const SyncClient);
    return d->daemonIf->property("HardwareID").toString();
}

QVariantMap SyncClient::userInfo() const
{
    Q_D(
    const SyncClient);
    return d->daemonIf->property("UserInfo").toMap();
}

QString SyncClient::gettext(const QString &str)
{
    return tr(str.toStdString().c_str());
}

void SyncClient::authCallback(const QVariantMap &tokenInfo)
{
    Q_D(SyncClient);
    qCDebug(deepinid_client) << tokenInfo;
    auto sessionID = tokenInfo.value("session_id").toString();
    auto clientID = tokenInfo.value("client_id").toString();
    auto code = tokenInfo.value("code").toString();
    auto state = tokenInfo.value("state").toString();

    Q_EMIT
    this->requestHide();

    auto region = tokenInfo.value("region").toString();
    auto syncUserID = tokenInfo.value("uid").toString();

    if (code.isEmpty()) {
        Q_EMIT
        this->onCancel(clientID);
        return;
    }

    d->session = tokenInfo;
    Q_EMIT
    this->onLogin(sessionID, clientID, code, state);
}

void SyncClient::open(const QString &url)
{
    QDesktopServices::openUrl(url);
}

void SyncClient::close()
{
    Q_EMIT
    this->prepareClose();
    qApp->quit();
}

void SyncClient::setProtocolCall(const bool &needCall)
{
    callLicenseDialog = needCall;
}

void SyncClient::JSLoadState(const bool isReady)
{
    if(isReady)
        Q_EMIT JSIsReady();
}

void SyncClient::setSession()
{
    Q_D(SyncClient);
    auto reply = d->daemonIf->Set(d->session);
    reply.waitForFinished();
    qDebug() << "set token with reply:" << reply.error();
}

void SyncClient::cleanSession()
{
    Q_D(SyncClient);
    auto reply = d->daemonIf->Set(QVariantMap());
    qDebug() << "clean token with reply:" << reply.error();
}

}
