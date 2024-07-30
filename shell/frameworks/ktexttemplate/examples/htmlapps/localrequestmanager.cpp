/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#define QT_DISABLE_DEPRECATED_BEFORE 0

#include "localrequestmanager.h"

#include <QDebug>
#include <QFile>

#include "templatereply.h"

LocalRequestManager::LocalRequestManager(KTextTemplate::Engine *engine, QObject *parent)
    : QNetworkAccessManager(parent)
    , m_engine(engine)
{
}

QNetworkReply *LocalRequestManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QUrl requestUrl = request.url();
    if (requestUrl.scheme() != "template") {
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }

    KTextTemplate::Template t = m_engine->loadByName(requestUrl.path());

    if (t->error()) {
        qDebug() << t->errorString();
    }

    QList<std::pair<QString, QString>> query = requestUrl.queryItems();
    if (outgoingData) {
        QUrl postData;
        postData.setEncodedQuery(outgoingData->readAll());
    }

    KTextTemplate::Context c;

    TemplateReply *reply = new TemplateReply(request, op, t, c);

    if (t->error()) {
        qDebug() << t->errorString();
    }
    reply->open(QIODevice::ReadOnly);

    return reply;
}

#include "moc_localrequestmanager.cpp"
