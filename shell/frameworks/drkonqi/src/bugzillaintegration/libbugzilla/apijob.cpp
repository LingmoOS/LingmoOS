/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "apijob.h"

#include <QMetaMethod>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include "bugzilla_debug.h"
#include "exceptions.h"

namespace Bugzilla
{
QJsonDocument APIJob::document() const
{
    ProtocolException::maybeThrow(this);
    Q_ASSERT(error() == KJob::NoError);

    auto document = QJsonDocument::fromJson(data());
    APIException::maybeThrow(document);
    return document;
}

QJsonObject APIJob::object() const
{
    return document().object();
}

void APIJob::setAutoStart(bool start)
{
    m_autostart = start;
}

void APIJob::connectNotify(const QMetaMethod &signal)
{
    if (m_autostart && signal == QMetaMethod::fromSignal(&KJob::finished)) {
        qCDebug(BUGZILLA_LOG) << "auto starting";
        start();
    }
    KJob::connectNotify(signal);
}

NetworkAPIJob::NetworkAPIJob(const QUrl &url, const std::function<QNetworkReply *(QNetworkAccessManager &, QNetworkRequest &)> &starter, QObject *parent)
    : APIJob(parent)
{
    m_manager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"_qba);
    request.setRawHeader("Accept"_qba, "application/json"_qba);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DrKonqi"_qba);

    auto reply = starter(m_manager, request);

    connect(reply, &QIODevice::readyRead, this, [this, reply] {
        m_data += reply->readAll();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();

        // Set errors, they are read by document() when the consumer reads
        // the data and possibly raised as exception.
        setError(reply->error());
        setErrorText(reply->errorString());

        // Force a delay on all API actions if configured. This allows
        // simulation of slow connections.
        static int delay = qEnvironmentVariableIntValue("DRKONQI_HTTP_DELAY_MS");
        if (delay > 0) {
            QTimer::singleShot(delay, this, [this] {
                emitResult();
            });
            return;
        }

        emitResult();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply] {
        setError(reply->error());
        setErrorText(reply->errorString());
    });
}

} // namespace Bugzilla

#include "moc_apijob.cpp"
