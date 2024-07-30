/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "servercontroller.h"

#include <rest/restapi.h>
#include <rest/restclient.h>

#include <core/product.h>
#include <core/survey.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QNetworkReply>
#include <QObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUuid>

#include <limits>

using namespace KUserFeedback::Console;

class SurveyApiTest : public QObject
{
    Q_OBJECT
private:
    ServerController m_server;

    ServerInfo testServer() const
    {
        ServerInfo s;
        s.setUrl(m_server.url());
        return s;
    }

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
        QStandardPaths::setTestModeEnabled(true);
        QVERIFY(m_server.start());
    }

    void init()
    {
        RESTClient client;
        client.setServerInfo(testServer());
        Product p;
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        auto reply = RESTApi::createProduct(&client, p);
        waitForFinished(reply);
    }

    void cleanup()
    {
        RESTClient client;
        client.setServerInfo(testServer());
        Product p;
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        auto reply = RESTApi::deleteProduct(&client, p);
        waitForFinished(reply);
    }

    void testSurveyCRUD()
    {
        RESTClient client;
        client.setServerInfo(testServer());
        client.setConnected(true);
        QVERIFY(client.isConnected());
        Product p;
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));

        // list surveys
        auto reply = RESTApi::listSurveys(&client, p);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QVERIFY(reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith(QLatin1String("text/json")));
        auto surveys = Survey::fromJson(reply->readAll());
        QVERIFY(surveys.isEmpty());

        // add new survey
        Survey s;
        s.setUuid(QUuid::createUuid());
        s.setName(QStringLiteral("unitTestSurvey"));
        s.setUrl(QUrl(QStringLiteral("http://www.kde.org")));
        s.setActive(false);
        s.setTarget(QStringLiteral("screen[0].dpi >= 200"));
        reply = RESTApi::createSurvey(&client, p, s);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // verify the new product is there
        reply = RESTApi::listSurveys(&client, p);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        surveys = Survey::fromJson(reply->readAll());
        QCOMPARE(surveys.size(), 1);
        QCOMPARE(surveys.at(0), s);

        // update a survey
        s.setActive(true);
        reply = RESTApi::updateSurvey(&client, s);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // verify the update worked
        reply = RESTApi::listSurveys(&client, p);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        surveys = Survey::fromJson(reply->readAll());
        QCOMPARE(surveys.size(), 1);
        QCOMPARE(s, surveys.at(0));

        // delete a survey
        reply = RESTApi::deleteSurvey(&client, s);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // verify it's gone
        reply = RESTApi::listSurveys(&client, p);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        surveys = Survey::fromJson(reply->readAll());
        QVERIFY(surveys.isEmpty());
    }

    void testInvalidSurveyOperations()
    {
        RESTClient client;
        client.setServerInfo(testServer());
        client.setConnected(true);
        QVERIFY(client.isConnected());

        Product invalidProduct;
        invalidProduct.setName(QStringLiteral("org.kde.UserFeedback.Invalid"));

        // create survey for non-existing product
        Survey s;
        s.setName(QStringLiteral("invalidTestSurvey"));
        s.setUrl(QUrl(QStringLiteral("http://www.kde.org")));
        auto reply = RESTApi::createSurvey(&client, invalidProduct, s);
        QVERIFY(waitForFinished(reply));
        QVERIFY(reply->error() != QNetworkReply::NoError);

        // update a non-existing survey
        s.setUuid(QUuid::createUuid());
        reply = RESTApi::updateSurvey(&client, s);
        QVERIFY(waitForFinished(reply));
        qDebug() << reply->readAll();
        QEXPECT_FAIL("", "check not implemented on the server", Continue);
        QVERIFY(reply->error() != QNetworkReply::NoError);

        // delete a non-existing survey
        reply = RESTApi::deleteSurvey(&client, s);
        QVERIFY(waitForFinished(reply));
        QEXPECT_FAIL("", "check not implemented on the server", Continue);
        QVERIFY(reply->error() != QNetworkReply::NoError);
    }
};

QTEST_MAIN(SurveyApiTest)

#include "surveyapitest.moc"

