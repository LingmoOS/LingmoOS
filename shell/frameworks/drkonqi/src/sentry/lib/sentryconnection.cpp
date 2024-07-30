// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include "sentryconnection.h"

SentryNetworkReply::SentryNetworkReply(QNetworkReply *reply, QObject *parent)
    : SentryReply(parent)
    , m_reply(reply)
{
    connect(reply, &QNetworkReply::finished, this, &SentryReply::finished);
}

SentryNetworkReply::~SentryNetworkReply()
{
    m_reply->deleteLater();
}

QByteArray SentryNetworkReply::readAll()
{
    return m_reply->readAll();
}

QNetworkReply::NetworkError SentryNetworkReply::error()
{
    return m_reply->error();
}

QString SentryNetworkReply::errorString()
{
    return m_reply->errorString();
}

SentryNetworkConnection::SentryNetworkConnection(QObject *parent)
    : SentryConnection(parent)
{
    m_manager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

SentryReply *SentryNetworkConnection::get(const QNetworkRequest &request)
{
    return new SentryNetworkReply(m_manager.get(request), this);
}

SentryReply *SentryNetworkConnection::post(const QNetworkRequest &request, const QByteArray &data)
{
    return new SentryNetworkReply(m_manager.post(request, data), this);
}

#include "moc_sentryconnection.cpp"
