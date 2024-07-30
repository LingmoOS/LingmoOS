// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#include "sentrypostman.h"

#include <chrono>
#include <thread>

#include <QDirIterator>
#include <QEventLoopLocker>
#include <QJsonDocument>

#include "debug.h"
#include "sentrypaths.h"

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

struct Transfer {
    QString path;
    QString sentPath;
    SentryReply *reply;
    std::shared_ptr<QEventLoopLocker> lock = std::make_shared<QEventLoopLocker>();
};

void SentryPostman::run()
{
    QEventLoopLocker lock;

    const auto cacheDir = SentryPaths::payloadsDir();
    if (cacheDir.isEmpty()) {
        qCWarning(SENTRY_DEBUG) << "Failed to resolve payloadsDir";
        return;
    }

    qCDebug(SENTRY_DEBUG) << "looking at " << cacheDir;
    QDirIterator it(cacheDir, QDir::Files);
    if (!it.hasNext()) {
        qCDebug(SENTRY_DEBUG) << "nothing there";
        return; // nothing to process
    }

    while (it.hasNext()) {
        const auto path = it.next();
        const auto filename = it.fileName();
        const auto mtime = it.fileInfo().lastModified();
        post(path, filename, mtime);
    }
}
void SentryPostman::post(const QString &path, const QString &filename, const QDateTime &mtime)
{
    const auto currentTime = QDateTime::currentDateTime();

    qCDebug(SENTRY_DEBUG) << "processing" << path;
    static constexpr auto daysInMonth = 30;
    if (currentTime - mtime >= daysInMonth * 24h) {
        // Incredibly old report. Probably not worth to submit it anymore. This can happen if the file is getting
        // rejected by the server over and over mostly.
        QFile::remove(path);
        return;
    }

    static constexpr auto flushTime = 8s; // arbitrary timeout in which we expect a flush to finish
    if (currentTime - mtime < flushTime) {
        std::this_thread::sleep_for(flushTime); // make sure the file is fully flushed
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(SENTRY_DEBUG) << "Failed to open" << path;
        return;
    }

    const auto doc = QJsonDocument::fromJson(file.readLine());
    file.reset();
    const auto dsn = doc["dsn"_L1].toString();
    if (dsn.isEmpty()) {
        qCWarning(SENTRY_DEBUG) << "Missing DSN. Discarding" << path;
        QFile::remove(path); // invalid, discard it
        return;
    }

    QNetworkRequest request(QUrl(dsn + "/envelope/"_L1));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-sentry-envelope"_L1);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DrKonqi"_L1);
    // Auth is handled through the payload itself, it should carry a DSN.
    qCDebug(SENTRY_DEBUG) << "requesting" << request.url();

    auto reply = m_connection.post(request, file.readAll());
    Transfer transfer{.path = path, .sentPath = SentryPaths::sentPayloadPath(filename), .reply = reply};
    QObject::connect(reply, &SentryReply::finished, this, [transfer] {
        transfer.reply->deleteLater();
        qCDebug(SENTRY_DEBUG) << transfer.reply->error();
        if (transfer.reply->error() != QNetworkReply::NoError) {
            qCWarning(SENTRY_DEBUG) << transfer.reply->error() << transfer.reply->errorString();
            return;
        }
        qCDebug(SENTRY_DEBUG) << "renaming" << transfer.path << "to" << transfer.sentPath;
        if (QFile::exists(transfer.sentPath)) {
            QFile::remove(transfer.sentPath);
        }
        QFile::rename(transfer.path, transfer.sentPath);
    });
}

#include "moc_sentrypostman.cpp"
