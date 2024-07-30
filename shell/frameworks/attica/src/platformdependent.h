/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>
    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PLATFORMDEPENDENT_H
#define ATTICA_PLATFORMDEPENDENT_H

#include <QList>
#include <QtPlugin>

class QByteArray;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;
class QUrl;

namespace Attica
{
class PlatformDependent
{
public:
    virtual ~PlatformDependent()
    {
    }
    virtual QList<QUrl> getDefaultProviderFiles() const = 0;
    virtual void addDefaultProviderFile(const QUrl &url) = 0;
    virtual void removeDefaultProviderFile(const QUrl &url) = 0;

    /**
     * Providers are enabled by default. Use this call to disable or enable them later.
     */
    virtual void enableProvider(const QUrl &baseUrl, bool enabled) const = 0;
    virtual bool isEnabled(const QUrl &baseUrl) const = 0;

    /**
     * Asks the PlatformDependent plugin if it have a credentials for baseUrl
     */
    virtual bool hasCredentials(const QUrl &baseUrl) const = 0;

    /**
     * Asks the PlatformDependent plugin to load credentials from storage,
     * and save it in user, password.
     */
    virtual bool loadCredentials(const QUrl &baseUrl, QString &user, QString &password) = 0;

    /**
     * Asks the PlatformDependent plugin to ask user to login and expects it to be stored in
     * user and password.
     * TODO KF6: Remove, askForCredentials and loadCredentials is essentially same, loadCredentials
     * can itself ask for credentials if it is not present in store.
     */
    virtual bool askForCredentials(const QUrl &baseUrl, QString &user, QString &password) = 0;

    /**
     * Method to be called by attica to store the user and password in storage used by PlatformDependent
     * plugin.
     * TODO KF6: Remove, if you let platform plugin ask for credentials, you don't want to save credentials
     * yourself and let plugin handle it.
     */
    virtual bool saveCredentials(const QUrl &baseUrl, const QString &user, const QString &password) = 0;

    virtual QNetworkReply *get(const QNetworkRequest &request) = 0;
    virtual QNetworkReply *post(const QNetworkRequest &request, QIODevice *data) = 0;
    virtual QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) = 0;
    virtual void setNam(QNetworkAccessManager *)
    {
    }
    virtual QNetworkAccessManager *nam() = 0;
};

}

Q_DECLARE_INTERFACE(Attica::PlatformDependent, "org.kde.Attica.Internals/1.2")

#endif
