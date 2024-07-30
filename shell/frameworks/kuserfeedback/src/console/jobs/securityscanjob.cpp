/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <kuserfeedback_version.h>

#include "securityscanjob.h"

#include <rest/restclient.h>

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

using namespace KUserFeedback::Console;

SecurityScanJob::SecurityScanJob(RESTClient* restClient, QObject* parent)
    : Job(parent)
    , m_restClient(restClient)
    , m_pendingPaths({
        QString(),
        QStringLiteral(".htaccess"),
        QStringLiteral("admin"),
        QStringLiteral("admin/index.php"),
        QStringLiteral("analytics"),
        QStringLiteral("analytics/index.php"),
        QStringLiteral("analytics/products"),
        QStringLiteral("config"),
        QStringLiteral("config/localconfig.php"),
        QStringLiteral("data"),
        QStringLiteral("receiver"),
        QStringLiteral("receiver/index.php"),
        QStringLiteral("shared"),
        QStringLiteral("shared/config.php"),
        QStringLiteral("shared/schema.json")
    })
{
    Q_ASSERT(m_restClient);
    Q_ASSERT(m_restClient->isConnected());

    processPending();
}

SecurityScanJob::~SecurityScanJob()
{
}

void SecurityScanJob::processPending()
{
    if (m_pendingPaths.isEmpty()) {
        Q_EMIT info(tr("No issues found."));
        emitFinished();
        return;
    }

    const auto command = m_pendingPaths.takeFirst();

    auto url = m_restClient->serverInfo().url();
    auto path = url.path();
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    path += command;
    url.setPath(path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString(QStringLiteral("UserFeedbackConsole/") + QStringLiteral(KUSERFEEDBACK_VERSION_STRING)));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif

    auto reply = m_restClient->networkAccessManager()->get(request);
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        const auto httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() == QNetworkReply::NoError && httpCode < 400) {
            qWarning() << reply->error() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            emitError(tr("Access to %1 is not protected!").arg(reply->request().url().toString()));
            return;
        }
        Q_EMIT info(tr("Access to %1 is protected (%2).").arg(reply->request().url().toString()).arg(httpCode));
        processPending();
    });
}

#include "moc_securityscanjob.cpp"
