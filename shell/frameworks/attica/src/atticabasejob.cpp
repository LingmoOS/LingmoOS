/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "atticabasejob.h"

#include <QAuthenticator>
#include <QDebug>
#include <QNetworkReply>
#include <QTimer>

#include "platformdependent.h"
#include "platformdependent_v3.h"
#include <attica_debug.h>
#include <atticautils.h>

using namespace Attica;

class Q_DECL_HIDDEN BaseJob::Private
{
public:
    Metadata m_metadata;
    PlatformDependent *m_internals;
    QPointer<QNetworkReply> m_reply;
    bool aborted{false};
    bool started = false;

    Private(PlatformDependent *internals)
        : m_internals(internals)
    {
    }

    bool redirection(QUrl &newUrl) const
    {
        if (m_reply == nullptr || m_reply->error() != QNetworkReply::NoError) {
            return false;
        }

        const int httpStatusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatusCode == 301 // Moved Permanently
            || httpStatusCode == 302 // Found
            || httpStatusCode == 303 // See Other
            || httpStatusCode == 307) { // Temporary Redirect
            QNetworkRequest request = m_reply->request();
            QUrl redirectUrl(m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
            if (redirectUrl.isRelative()) {
                QUrl baseUrl(request.url());
                newUrl = baseUrl.resolved(redirectUrl);
                qCDebug(ATTICA) << "resolving relative URL redirection to" << newUrl.toString();
            } else {
                newUrl = redirectUrl;
                qCDebug(ATTICA) << "resolving absolute URL redirection to" << newUrl.toString();
            }
            return true;
        }

        return false;
    }
};

BaseJob::BaseJob(PlatformDependent *internals)
    : d(std::make_unique<Private>(internals))
{
}

BaseJob::~BaseJob() = default;

void BaseJob::dataFinished()
{
    if (!d->m_reply) {
        return;
    }

    bool error = d->m_reply->error() != QNetworkReply::NoError && d->m_reply->error() != QNetworkReply::OperationCanceledError;

    // handle redirections automatically
    QUrl newUrl;
    if (d->redirection(newUrl)) {
        // qCDebug(ATTICA) << "BaseJob::dataFinished" << newUrl;
        QNetworkRequest request = d->m_reply->request();
        QNetworkAccessManager::Operation operation = d->m_reply->operation();
        if (newUrl.isValid() && operation == QNetworkAccessManager::GetOperation) {
            d->m_reply->deleteLater();
            // reissue same request with different Url
            request.setUrl(newUrl);
            d->m_reply = internals()->get(request);
            connect(d->m_reply, &QNetworkReply::finished, this, &BaseJob::dataFinished);
            return;
        }
        error = true;
    }

    if (error) {
        d->m_metadata.setError(Metadata::NetworkError);
        d->m_metadata.setStatusCode(d->m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        d->m_metadata.setStatusString(d->m_reply->errorString());
        d->m_metadata.setHeaders(d->m_reply->rawHeaderPairs());
    } else if (d->m_reply->error() == QNetworkReply::OperationCanceledError) {
        d->m_metadata.setError(Metadata::NoError);
    } else {
        QByteArray data = d->m_reply->readAll();
        // qCDebug(ATTICA) << "XML Returned:\n" << data;
        parse(QString::fromUtf8(data.constData()));
        if (d->m_metadata.statusCode() >= 100 && d->m_metadata.statusCode() < 200) {
            d->m_metadata.setError(Metadata::NoError);
        } else {
            d->m_metadata.setError(Metadata::OcsError);
        }
    }
    Q_EMIT finished(this);

    d->m_reply->deleteLater();
    deleteLater();
}

void BaseJob::start()
{
    if (!d->started) {
        d->started = true;
        QTimer::singleShot(0, this, &BaseJob::doWork);
    }
}

void BaseJob::doWork()
{
    if (d->aborted) {
        return;
    }

    auto platformDependentV3 = dynamic_cast<Attica::PlatformDependentV3 *>(d->m_internals);
    if (platformDependentV3 && !platformDependentV3->isReady()) {
        connect(platformDependentV3, &Attica::PlatformDependentV3::readyChanged, this, &BaseJob::doWork);
        return;
    }

    d->m_reply = executeRequest();
    qCDebug(ATTICA) << "executing" << Utils::toString(d->m_reply->operation()) << "request for" << d->m_reply->url();
    connect(d->m_reply, &QNetworkReply::finished, this, &BaseJob::dataFinished);
    connect(d->m_reply->manager(), &QNetworkAccessManager::authenticationRequired, this, &BaseJob::authenticationRequired);
    connect(d->m_reply, &QNetworkReply::errorOccurred, [](QNetworkReply::NetworkError code) {
        qCDebug(ATTICA) << "error found" << code;
    });
}

void BaseJob::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    auth->setUser(reply->request().attribute((QNetworkRequest::Attribute)BaseJob::UserAttribute).toString());
    auth->setPassword(reply->request().attribute((QNetworkRequest::Attribute)BaseJob::PasswordAttribute).toString());
}

void BaseJob::abort()
{
    d->aborted = true;
    if (d->m_reply) {
        d->m_reply->abort();
        d->m_reply->deleteLater();
    }
    deleteLater();
}

bool BaseJob::isAborted() const
{
    return d->aborted;
}

PlatformDependent *BaseJob::internals()
{
    return d->m_internals;
}

Metadata BaseJob::metadata() const
{
    return d->m_metadata;
}

void BaseJob::setMetadata(const Attica::Metadata &data) const
{
    d->m_metadata = data;
}

#include "moc_atticabasejob.cpp"
