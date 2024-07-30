/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "servercontroller.h"

#include <rest/restapi.h>
#include <rest/restclient.h>

#include <provider/core/auditloguicontroller.h>
#include <provider/core/provider.h>
#include <provider/core/platforminfosource.h>
#include <provider/core/screeninfosource.h>

#include <core/product.h>
#include <core/schemaentryelement.h>
#include <core/schemaentrytemplates.h>

#include <QAbstractItemModel>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest/qtest.h>
#include <QNetworkReply>
#include <QObject>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KUserFeedback;
using namespace KUserFeedback::Console;

class SubmitTest : public QObject
{
    Q_OBJECT
private:
    ServerController m_server;

    bool waitForFinished(QNetworkReply *reply)
    {
        Q_ASSERT(reply);
        QSignalSpy spy(reply, &QNetworkReply::finished);
        Q_ASSERT(spy.isValid());
        return spy.wait();
    }

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(schematemplates);
        QStandardPaths::setTestModeEnabled(true);
        QVERIFY(m_server.start());
    }

    void init()
    {
        // clear past audit logs
        AuditLogUiController alc;
        alc.clear();
        QVERIFY(alc.logEntryModel());
        QCOMPARE(alc.logEntryModel()->rowCount(), 0);
    }

    void testProvider_data()
    {
        QTest::addColumn<QString>("path");
        QTest::newRow("direct") << QString();
        QTest::newRow("absolute redirect") << QStringLiteral("/absRedirect");
        QTest::newRow("relative redirect") << QStringLiteral("/relRedirect");
    }

    void testProvider()
    {
        QFETCH(QString, path);
        ServerInfo serverInfo;
        auto serverUrl = m_server.url();
        serverInfo.setUrl(serverUrl);
        serverUrl.setPath(path);

        // delete previous leftovers
        RESTClient client;
        client.setServerInfo(serverInfo);
        client.setConnected(true);
        QVERIFY(client.isConnected());
        Product p;
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        auto reply = RESTApi::deleteProduct(&client, p);
        waitForFinished(reply);

        // create test product
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        QVERIFY(p.isValid());
        reply = RESTApi::createProduct(&client, p);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // submit data
        Provider provider;
        provider.setProductIdentifier(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        provider.setTelemetryMode(Provider::DetailedUsageStatistics);
        provider.setFeedbackServer(serverUrl);
        provider.addDataSource(new ScreenInfoSource);
        provider.addDataSource(new PlatformInfoSource);
        provider.submit();
        QTest::qWait(100); // HACK submit is async

        // retrieve data
        reply = RESTApi::listSamples(&client, p);
        QVERIFY(waitForFinished(reply));
        QVERIFY(reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith(QLatin1String("text/json")));
        auto doc = QJsonDocument::fromJson(reply->readAll());
        QVERIFY(doc.isArray());
        auto a = doc.array();
        QCOMPARE(a.size(), 1);

        auto obj = a.at(0).toObject();
        QVERIFY(!obj.isEmpty());
        QVERIFY(obj.contains(QLatin1String("id")));
        QVERIFY(obj.contains(QLatin1String("timestamp")));

        QVERIFY(obj.contains(QLatin1String("platform")));
        auto sub = obj.value(QLatin1String("platform")).toObject();
        QVERIFY(sub.contains(QLatin1String("os")));
        QVERIFY(sub.contains(QLatin1String("version")));

        QVERIFY(obj.contains(QLatin1String("screens")));
        a = obj.value(QLatin1String("screens")).toArray();
        QVERIFY(a.size() > 0);
        sub = a.at(0).toObject();
        QVERIFY(sub.contains(QLatin1String("height")));
        QVERIFY(sub.contains(QLatin1String("width")));

        // check we wrote an audit log
        AuditLogUiController alc;
        QVERIFY(alc.logEntryModel());
        QCOMPARE(alc.logEntryModel()->rowCount(), 1);
    }

    void testRedirectLoop()
    {
        auto serverUrl = m_server.url();
        serverUrl.setPath(QLatin1String("/circleRedirect"));

        // this must pass without ending in an infinite loop
        Provider provider;
        provider.setProductIdentifier(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        provider.setTelemetryMode(Provider::DetailedUsageStatistics);
        provider.setFeedbackServer(serverUrl);
        provider.addDataSource(new ScreenInfoSource);
        provider.addDataSource(new PlatformInfoSource);
        provider.submit();
        QTest::qWait(500); // HACK submit is async, and we have no way of knowning the redirect loop ended...
    }

    void testSurveyWithoutTelemetry()
    {
        ServerInfo serverInfo;
        auto serverUrl = m_server.url();
        serverInfo.setUrl(serverUrl);

        // delete previous leftovers
        RESTClient client;
        client.setServerInfo(serverInfo);
        client.setConnected(true);
        QVERIFY(client.isConnected());
        Product p;
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        auto reply = RESTApi::deleteProduct(&client, p);
        waitForFinished(reply);

        // create test product
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        QVERIFY(p.isValid());
        reply = RESTApi::createProduct(&client, p);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // submit data
        Provider provider;
        provider.setProductIdentifier(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        provider.setTelemetryMode(Provider::NoTelemetry);
        provider.setSubmissionInterval(7);
        provider.setSurveyInterval(30);
        provider.setFeedbackServer(serverUrl);
        provider.addDataSource(new ScreenInfoSource);
        provider.addDataSource(new PlatformInfoSource);
        provider.submit();
        QTest::qWait(100); // HACK submit is async

        // retrieve data
        reply = RESTApi::listSamples(&client, p);
        QVERIFY(waitForFinished(reply));
        QVERIFY(reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith(QLatin1String("text/json")));
        auto doc = QJsonDocument::fromJson(reply->readAll());
        QVERIFY(doc.isArray());
        auto a = doc.array();
        QCOMPARE(a.size(), 0);

        // check we wrote an audit log
        AuditLogUiController alc;
        QVERIFY(alc.logEntryModel());
        QCOMPARE(alc.logEntryModel()->rowCount(), 1);
    }
};

QTEST_MAIN(SubmitTest)

#include "submittest.moc"
