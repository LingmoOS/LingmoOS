// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <sentrydsns.h>

#include "sentryfilereply.h"

using namespace Qt::StringLiterals;

class FileConnection : public SentryConnection
{
public:
    using SentryConnection::SentryConnection;

    SentryReply *get(const QNetworkRequest &request) override
    {
        if (request.url() == QUrl("https://autoconfig.kde.org/drkonqi/sentry/0/dsns.json"_L1)) {
            return new FileReply(QFINDTESTDATA("data/dsns.json"));
        }

        qWarning() << "unhandled request" << request.url();
        Q_ASSERT(false);
        return nullptr;
    }

    SentryReply *post(const QNetworkRequest &request, const QByteArray &) override
    {
        qWarning() << "unhandled request" << request.url();
        Q_ASSERT(false);
        return nullptr;
    }
};

class NoopFileConnection : public FileConnection
{
public:
    using FileConnection::FileConnection;

    SentryReply *get(const QNetworkRequest &) override
    {
        auto reply = new FileReply("/dev/null"_L1);
        reply->m_error = QNetworkReply::HostNotFoundError;
        reply->m_errorString = "Yada yada"_L1;
        return reply;
    }
};

class SentryDSNsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testKWrite()
    {
        SentryDSNs dsns(std::make_shared<FileConnection>(), SentryDSNs::Cache::No);
        QSignalSpy spy(&dsns, &SentryDSNs::loaded);
        dsns.load();
        spy.wait();
        const auto context = dsns.context("kwrite"_L1);
        QCOMPARE(context.index, "3"_L1);
        QCOMPARE(context.key, "ba18a003921d4ac281851019f69050b4"_L1);
        QCOMPARE(context.project, "kwrite"_L1);
        QCOMPARE(context.dsnUrl(), QUrl("https://ba18a003921d4ac281851019f69050b4@crash-reports.kde.org/api/3"_L1));
        QCOMPARE(context.envelopeUrl(), QUrl("https://ba18a003921d4ac281851019f69050b4@crash-reports.kde.org/api/3/envelope/"_L1));
    }

    void testFallthrough()
    {
        SentryDSNs dsns(std::make_shared<FileConnection>(), SentryDSNs::Cache::No);
        QSignalSpy spy(&dsns, &SentryDSNs::loaded);
        dsns.load();
        spy.wait();
        const auto context = dsns.context("does_not_exist"_L1);
        QCOMPARE(context.index, "12"_L1);
        QCOMPARE(context.key, "asdf"_L1);
        QCOMPARE(context.project, "fallthrough"_L1);
    }

    void testBuiltinFallthrough()
    {
        SentryDSNs dsns(std::make_shared<NoopFileConnection>(), SentryDSNs::Cache::No);
        QSignalSpy spy(&dsns, &SentryDSNs::loaded);
        dsns.load();
        spy.wait();
        const auto context = dsns.context("does_not_exist"_L1);
        QCOMPARE(context.index, "11"_L1);
        QCOMPARE(context.key, "456f53a71a074438bbb786d6add63241"_L1);
        QCOMPARE(context.project, "fallthrough"_L1);
    }
};

QTEST_GUILESS_MAIN(SentryDSNsTest)

#include "sentrydsnstest.moc"
