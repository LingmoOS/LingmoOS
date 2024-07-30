/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>
    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>
    SPDX-FileCopyrightText: 2012 Jeff Mitchell <mitchell@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_QTPLATFORMDEPENDENT_P_H
#define ATTICA_QTPLATFORMDEPENDENT_P_H

#include "platformdependent_v3.h"

#include <QHash>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QSet>
#include <QThread>

namespace Attica
{
class QtPlatformDependent : public Attica::PlatformDependentV3
{
public:
    QtPlatformDependent();
    ~QtPlatformDependent() override;

    void setNam(QNetworkAccessManager *nam) override;
    QNetworkAccessManager *nam() override;

    QList<QUrl> getDefaultProviderFiles() const override;
    void addDefaultProviderFile(const QUrl &url) override;
    void removeDefaultProviderFile(const QUrl &url) override;
    void enableProvider(const QUrl &baseUrl, bool enabled) const override;
    bool isEnabled(const QUrl &baseUrl) const override;

    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *post(const QNetworkRequest &request, QIODevice *data) override;
    QNetworkReply *get(const QNetworkRequest &request) override;
    bool hasCredentials(const QUrl &baseUrl) const override;
    bool saveCredentials(const QUrl &baseUrl, const QString &user, const QString &password) override;
    bool loadCredentials(const QUrl &baseUrl, QString &user, QString &password) override;
    bool askForCredentials(const QUrl &baseUrl, QString &user, QString &password) override;
    QNetworkReply *deleteResource(const QNetworkRequest &request) override;
    QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *put(const QNetworkRequest &request, QIODevice *data) override;
    [[nodiscard]] bool isReady() override;

private:
    QMutex m_accessMutex;
    QHash<QThread *, QNetworkAccessManager *> m_threadNamHash;
    QSet<QThread *> m_ourNamSet;
    QHash<QString, QPair<QString, QString>> m_passwords;
};

}

#endif
