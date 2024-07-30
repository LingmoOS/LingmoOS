// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QJsonDocument>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <sentrypostbox.h>

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
        if (request.url() == QUrl("https://456f53a71a074438bbb786d6add63241@crash-reports.kde.org/api/11/envelope/"_L1)) {
            return new FileReply("/dev/null"_L1);
        }

        qWarning() << "unhandled request" << request.url();
        Q_ASSERT(false);
        return nullptr;
    }
};

class SentryPostboxTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testKWrite()
    {
        QFile eventPayloadFile(QFINDTESTDATA("data/sentry-event.json"));
        QVERIFY(eventPayloadFile.open(QFile::ReadOnly));

        SentryPostbox box("kwrite"_L1, std::make_shared<FileConnection>());
        box.addEventPayload(QJsonDocument::fromJson(eventPayloadFile.readAll()));
        QSignalSpy spy(&box, &SentryPostbox::hasDeliveredChanged);
        box.deliver();
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QVERIFY(box.hasDelivered());
    }

    void testFallthrough()
    {
        QFile eventPayloadFile(QFINDTESTDATA("data/sentry-event.json"));
        QVERIFY(eventPayloadFile.open(QFile::ReadOnly));

        SentryPostbox box("foobar_does_not_exist_in_dsns"_L1, std::make_shared<FileConnection>());
        box.addEventPayload(QJsonDocument::fromJson(eventPayloadFile.readAll()));
        QSignalSpy spy(&box, &SentryPostbox::hasDeliveredChanged);
        box.deliver();
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QVERIFY(box.hasDelivered());
    }
};

QTEST_GUILESS_MAIN(SentryPostboxTest)

#include "sentrypostboxtest.moc"
