// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "session.h"

#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>

#include "utils/utils.h"
#include "ipc/const.h"
#include "ipc/deepinid_interface.h"

namespace ddc
{

QNetworkAccessManager manager;

class SessionPrivate
{
    explicit SessionPrivate(Session *parent)
        : q_ptr(parent)
    {
        daemonIf = new DeepinIDInterface(Const::SyncDaemonService,
                                         Const::SyncDaemonPath,
                                         QDBusConnection::sessionBus());
    }

    DeepinIDInterface *daemonIf = nullptr;

    Session *q_ptr;
    Q_DECLARE_PUBLIC(Session)
};

Session::Session(QObject *parent)
    : QObject(parent), dd_ptr(new SessionPrivate(this))
{

}

void Session::authorize(const AuthorizeRequest &authReq)
{
    Q_D(Session);
    auto url = utils::authCodeURL("/oauth2/client/authorize",
                                  authReq.clientID,
                                  authReq.scopes,
                                  authReq.callback,
                                  authReq.state);
    // get from dbus
    QDBusReply<QByteArray> dbusReply = d->daemonIf->Get();
    auto sessionJson = dbusReply.value();
    auto doc = QJsonDocument::fromJson(sessionJson);
    auto session = doc.object();
    auto sessionID = session.value("SessionID").toString();
    QUrl::toPercentEncoding(url);
    QNetworkRequest req(url);
    req.setRawHeader("X-DeepinID-SessionID", sessionID.toLatin1());
    QTimer *timer = new QTimer(this);
    auto reply = manager.get(req);
    timer->start(3000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=] {
        qDebug() << "timeout";
        if (reply->isRunning()) {
            reply->abort();
        }
    });

    qDebug() << "get" << req.url() << req.rawHeader("X-DeepinID-SessionID");
    connect(reply, &QNetworkReply::finished, this, [=]()
    {
        timer->stop();
        AuthorizeResponse resp;
        resp.success = true;
        resp.req = authReq;

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << reply->error();
            resp.success = false;
        }
        else {
            auto data = reply->readAll();
            qDebug() << "resp" << data;
            auto json = QJsonDocument::fromJson(data).object();
            resp.code = json.value("code").toString();
            resp.state = authReq.state;

            if (resp.code.isEmpty()) {
                resp.success = false;
            }
        }
        qDebug() << "resp" << resp.success;
        Q_EMIT this->authorizeFinished(resp);
    });
}

Session::~Session() = default;

}

