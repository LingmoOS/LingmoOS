/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "handshakejob.h"

#include <rest/restapi.h>
#include <rest/restclient.h>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

using namespace KUserFeedback::Console;

HandshakeJob::HandshakeJob(RESTClient* restClient, QObject* parent)
    : Job(parent)
    , m_restClient(restClient)
{
    auto reply = RESTApi::checkSchema(restClient);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            Q_EMIT info(tr("Connected to %1.").arg(m_restClient->serverInfo().url().toString()));
            processResponse(reply);
        } else {
            emitError(reply->errorString());
        }
        reply->deleteLater();
    });
    connect(reply, &QNetworkReply::redirected, this, [this](const auto &url) {
        auto s = m_restClient->serverInfo();
        auto u = url;
        auto p = u.path();
        p.remove(QLatin1String("analytics/check_schema"));
        u.setPath(p);
        s.setUrl(u);
        m_restClient->setServerInfo(s);
    });
}

HandshakeJob::~HandshakeJob()
{
}

void HandshakeJob::processResponse(QNetworkReply* reply)
{
    const auto doc = QJsonDocument::fromJson(reply->readAll());
    const auto obj = doc.object();

    const auto protoVer = obj.value(QLatin1String("protocolVersion")).toInt();
    if (protoVer != 2) {
        emitError(tr("Incompatible protocol: %1.").arg(protoVer));
        return;
    }

    const auto prevSchema = obj.value(QLatin1String("previousSchemaVersion")).toInt();
    const auto curSchema = obj.value(QLatin1String("currentSchemaVersion")).toInt();
    if (prevSchema != curSchema)
        Q_EMIT info(tr("Updated database schema from version %1 to %2.").arg(prevSchema).arg(curSchema));

    m_restClient->setConnected(true);
    emitFinished();
}

#include "moc_handshakejob.cpp"
