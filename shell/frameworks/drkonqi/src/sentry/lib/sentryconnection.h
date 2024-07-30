// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <memory>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

// Represents a NetworkReply. Implemented for QNetwork, and testing mocks.
class SentryReply : public QObject
{
    Q_OBJECT
public:
    virtual QByteArray readAll() = 0;
    virtual QNetworkReply::NetworkError error() = 0;
    virtual QString errorString() = 0;

Q_SIGNALS:
    void finished();

protected:
    using QObject::QObject;
};

// Represents a NetworkAccessManager. Implemented for QNetwork, and testing mocks.
class SentryConnection : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual SentryReply *get(const QNetworkRequest &request) = 0;
    virtual SentryReply *post(const QNetworkRequest &request, const QByteArray &data) = 0;
};

// A QNetworkReply based reply implementation.
class SentryNetworkReply final : public SentryReply
{
public:
    explicit SentryNetworkReply(QNetworkReply *reply, QObject *parent = nullptr);
    ~SentryNetworkReply() final;
    Q_DISABLE_COPY_MOVE(SentryNetworkReply)

    QByteArray readAll() final;
    QNetworkReply::NetworkError error() final;
    QString errorString() final;

private:
    QNetworkReply *m_reply;
};

// A QNetworkAccessManager based connection implementation.
class SentryNetworkConnection final : public SentryConnection
{
public:
    explicit SentryNetworkConnection(QObject *parent = nullptr);

    SentryReply *get(const QNetworkRequest &request) final;
    SentryReply *post(const QNetworkRequest &request, const QByteArray &data) final;

private:
    QNetworkAccessManager m_manager;
};
