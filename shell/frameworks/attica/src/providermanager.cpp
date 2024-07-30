/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "providermanager.h"

#include "attica_debug.h"
#include "atticautils.h"

#include <QAuthenticator>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QNetworkProxy>
#include <QPluginLoader>
#include <QSet>
#include <QSignalMapper>
#include <QTimer>
#include <QXmlStreamReader>

#include "platformdependent.h"
#include "platformdependent_v3.h"
#include "qtplatformdependent_p.h"
#include <QLibraryInfo>

using namespace Attica;

class Q_DECL_HIDDEN ProviderManager::Private
{
public:
    PlatformDependent *m_internals;
    QHash<QUrl, Provider> m_providers;
    QHash<QUrl, QUrl> m_providerTargets;
    QHash<QString, QNetworkReply *> m_downloads;
    bool m_authenticationSuppressed;

    Private()
        : m_internals(nullptr)
        , m_authenticationSuppressed(false)
    {
    }
    ~Private()
    {
        // do not delete m_internals: it is the root component of a plugin!
    }
};

PlatformDependent *ProviderManager::loadPlatformDependent(const ProviderFlags &flags)
{
    if (flags & ProviderManager::DisablePlugins) {
        return new QtPlatformDependent;
    }

    QPluginLoader loader(QStringLiteral("attica_kde"));
    PlatformDependent *ret = qobject_cast<PlatformDependent *>(loader.instance());

    return ret ? ret : new QtPlatformDependent;
}

ProviderManager::ProviderManager(const ProviderFlags &flags)
    : d(new Private)
{
    d->m_internals = loadPlatformDependent(flags);
    connect(d->m_internals->nam(), &QNetworkAccessManager::authenticationRequired, this, &ProviderManager::authenticate);
}

void ProviderManager::loadDefaultProviders()
{
    auto platformDependentV3 = dynamic_cast<Attica::PlatformDependentV3 *>(d->m_internals);
    if (platformDependentV3 && !platformDependentV3->isReady()) {
        connect(platformDependentV3,
                &Attica::PlatformDependentV3::readyChanged,
                this,
                &ProviderManager::slotLoadDefaultProvidersInternal,
                Qt::QueuedConnection);
        return;
    }

    QTimer::singleShot(0, this, &ProviderManager::slotLoadDefaultProvidersInternal);
}

void ProviderManager::setAuthenticationSuppressed(bool suppressed)
{
    d->m_authenticationSuppressed = suppressed;
}

void ProviderManager::clear()
{
    d->m_providerTargets.clear();
    d->m_providers.clear();
}

void ProviderManager::slotLoadDefaultProvidersInternal()
{
    const auto providerFiles = d->m_internals->getDefaultProviderFiles();
    for (const QUrl &url : providerFiles) {
        addProviderFile(url);
    }
    if (d->m_downloads.isEmpty()) {
        Q_EMIT defaultProvidersLoaded();
    }
}

QList<QUrl> ProviderManager::defaultProviderFiles()
{
    return d->m_internals->getDefaultProviderFiles();
}

ProviderManager::~ProviderManager()
{
    delete d;
}

void ProviderManager::addProviderFileToDefaultProviders(const QUrl &url)
{
    d->m_internals->addDefaultProviderFile(url);
    addProviderFile(url);
}

void ProviderManager::removeProviderFileFromDefaultProviders(const QUrl &url)
{
    d->m_internals->removeDefaultProviderFile(url);
}

void ProviderManager::addProviderFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "ProviderManager::addProviderFile: could not open provider file: " << url.toString();
            return;
        }
        parseProviderFile(QLatin1String(file.readAll()), url);
    } else {
        if (!d->m_downloads.contains(url.toString())) {
            QNetworkRequest req(url);
            req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
            QNetworkReply *reply = d->m_internals->get(req);
            qCDebug(ATTICA) << "executing" << Utils::toString(reply->operation()) << "for" << reply->url();
            connect(reply, &QNetworkReply::finished, this, [this, url]() {
                fileFinished(url.toString());
            });
            d->m_downloads.insert(url.toString(), reply);
        }
    }
}

void ProviderManager::fileFinished(const QString &url)
{
    QNetworkReply *reply = d->m_downloads.take(url);
    if (reply) {
        if (reply->error()) {
            Q_EMIT failedToLoad(QUrl(url), reply->error());
        } else {
            parseProviderFile(QLatin1String(reply->readAll()), QUrl(url));
        }
        reply->deleteLater();
    } else {
        Q_EMIT failedToLoad(QUrl(url), QNetworkReply::UnknownNetworkError);
    }
}

void ProviderManager::addProviderFromXml(const QString &providerXml)
{
    parseProviderFile(providerXml, QUrl());
}

void ProviderManager::parseProviderFile(const QString &xmlString, const QUrl &url)
{
    QXmlStreamReader xml(xmlString);
    while (!xml.atEnd() && xml.readNext()) {
        if (xml.isStartElement() && xml.name() == QLatin1String("provider")) {
            QUrl baseUrl;
            QString name;
            QUrl icon;
            QString person;
            QString friendV;
            QString message;
            QString achievement;
            QString activity;
            QString content;
            QString fan;
            QString forum;
            QString knowledgebase;
            QString event;
            QString comment;
            QString registerUrl;

            while (!xml.atEnd() && xml.readNext()) {
                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("location")) {
                        baseUrl = QUrl(xml.readElementText());
                    } else if (xml.name() == QLatin1String("name")) {
                        name = xml.readElementText();
                    } else if (xml.name() == QLatin1String("icon")) {
                        icon = QUrl(xml.readElementText());
                    } else if (xml.name() == QLatin1String("person")) {
                        person = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("friend")) {
                        friendV = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("message")) {
                        message = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("achievement")) {
                        achievement = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("activity")) {
                        activity = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("content")) {
                        content = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("fan")) {
                        fan = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("forum")) {
                        forum = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("knowledgebase")) {
                        knowledgebase = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("event")) {
                        event = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("comment")) {
                        comment = xml.attributes().value(QLatin1String("ocsversion")).toString();
                    } else if (xml.name() == QLatin1String("register")) {
                        registerUrl = xml.readElementText();
                    }
                } else if (xml.isEndElement() && xml.name() == QLatin1String("provider")) {
                    break;
                }
            }
            if (!baseUrl.isEmpty()) {
                // qCDebug(ATTICA) << "Adding provider" << baseUrl;
                d->m_providers.insert(baseUrl,
                                      Provider(d->m_internals,
                                               baseUrl,
                                               name,
                                               icon,
                                               person,
                                               friendV,
                                               message,
                                               achievement,
                                               activity,
                                               content,
                                               fan,
                                               forum,
                                               knowledgebase,
                                               event,
                                               comment,
                                               registerUrl));
                d->m_providerTargets[url] = baseUrl;
                Q_EMIT providerAdded(d->m_providers.value(baseUrl));
            }
        }
    }

    if (xml.error() != QXmlStreamReader::NoError) {
        qCDebug(ATTICA) << "error:" << xml.errorString() << "in" << url;
    }

    if (d->m_downloads.isEmpty()) {
        Q_EMIT defaultProvidersLoaded();
    }
}

Provider ProviderManager::providerFor(const QUrl &url) const
{
    return providerByUrl(d->m_providerTargets.value(url));
}

Provider ProviderManager::providerByUrl(const QUrl &url) const
{
    return d->m_providers.value(url);
}

QList<Provider> ProviderManager::providers() const
{
    return d->m_providers.values();
}

QList<QUrl> ProviderManager::providerFiles() const
{
    return d->m_providerTargets.keys();
}

void ProviderManager::authenticate(QNetworkReply *reply, QAuthenticator *auth)
{
    QUrl baseUrl;
    const QList<QUrl> urls = d->m_providers.keys();
    for (const QUrl &url : urls) {
        if (url.isParentOf(reply->url())) {
            baseUrl = url;
            break;
        }
    }

    // qCDebug(ATTICA) << "ProviderManager::authenticate" << baseUrl;

    QString user;
    QString password;
    if (auth->user().isEmpty() && auth->password().isEmpty()) {
        if (d->m_internals->hasCredentials(baseUrl)) {
            if (d->m_internals->loadCredentials(baseUrl, user, password)) {
                // qCDebug(ATTICA) << "ProviderManager::authenticate: loading authentication";
                auth->setUser(user);
                auth->setPassword(password);
                return;
            }
        }
    }

    if (!d->m_authenticationSuppressed && d->m_internals->askForCredentials(baseUrl, user, password)) {
        // qCDebug(ATTICA) << "ProviderManager::authenticate: asking internals for new credentials";
        // auth->setUser(user);
        // auth->setPassword(password);
        return;
    }

    qWarning() << "ProviderManager::authenticate: No authentication credentials provided, aborting." << reply->url().toString();
    Q_EMIT authenticationCredentialsMissing(d->m_providers.value(baseUrl));
    reply->abort();
}

void ProviderManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
    Q_UNUSED(proxy)
    Q_UNUSED(authenticator)
}

#include "moc_providermanager.cpp"
