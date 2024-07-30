// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <memory>

#include <QHash>
#include <QObject>

class SentryConnection;

// A SentryDSNContext is KDE-specific data blob. It defaults to a hardcoded builtin variant of the fallthrough DSN.
struct SentryDSNContext {
    QString project = u"fallthrough"_qs;
    QString key = u"456f53a71a074438bbb786d6add63241"_qs;
    QString index = u"11"_qs;

    QUrl dsnUrl() const;
    QUrl envelopeUrl() const;
};

// Manages retrieval of DSNContexts from the sentry server.
class SentryDSNs : public QObject
{
    Q_OBJECT
public:
    enum class Cache { Yes, No };
    explicit SentryDSNs(std::shared_ptr<SentryConnection> connection, Cache cache = Cache::Yes, QObject *parent = nullptr);
    void load();
    // The context for the specified applicationName (aka project). This may return the fallthrough DSN if
    // no mapping was found.
    SentryDSNContext context(const QString &applicationName);

Q_SIGNALS:
    void loaded();

private:
    void loadData(const QByteArray &data);
    std::shared_ptr<SentryConnection> m_connection;
    bool m_loaded = false;
    QHash<QString, SentryDSNContext> m_applicationContexts;
    QString m_cachePath;
    bool m_cache;
};
