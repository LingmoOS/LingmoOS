/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <provider.h>
#include <platforminfosource.h>
#include <screeninfosource.h>
#include <startcountsource.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KUserFeedback;

class ProviderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QCoreApplication::setApplicationName(QStringLiteral("providertest"));
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback"));;
        s.beginGroup(QLatin1String("UserFeedback"));
        s.remove(QLatin1String("LastEncouragement"));
        s.remove(QLatin1String("Enabled"));
    }

    void testProductId()
    {
        Provider p;
        QCOMPARE(p.productIdentifier(), QLatin1String("org.kde.providertest"));
    }

    void testNoTelemetry()
    {
        Provider provider;
        provider.setProductIdentifier(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));
        provider.addDataSource(new ScreenInfoSource);
        provider.addDataSource(new PlatformInfoSource);
        provider.setTelemetryMode(Provider::NoTelemetry);
        QByteArray b;
        QMetaObject::invokeMethod(&provider, "jsonData", Q_RETURN_ARG(QByteArray, b), Q_ARG(KUserFeedback::Provider::TelemetryMode, provider.telemetryMode()));
        b.replace('\n', "");
        QCOMPARE(b.constData(), "{}");
    }

    void testLoadStore()
    {
        {
            Provider p;
            p.setTelemetryMode(Provider::NoTelemetry);
            p.setSurveyInterval(-1);
        }

        {
            Provider p;
            QCOMPARE(p.telemetryMode(), Provider::NoTelemetry);
            QCOMPARE(p.surveyInterval(), -1);
            p.setTelemetryMode(Provider::DetailedSystemInformation);
            p.setSurveyInterval(90);
        }

        {
            Provider p;
            QCOMPARE(p.telemetryMode(), Provider::DetailedSystemInformation);
            QCOMPARE(p.surveyInterval(), 90);
        }
    }

    void testEncouragement()
    {
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastEncouragement"));
        }

        {
            Provider p;
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            QVERIFY(!spy.wait(10));
            p.setApplicationStartsUntilEncouragement(0);
            p.setApplicationUsageTimeUntilEncouragement(0);
            QVERIFY(spy.wait(10));
            QCOMPARE(spy.count(), 1);
        }

        {
            Provider p;
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setApplicationUsageTimeUntilEncouragement(0);
            QVERIFY(!spy.wait(10));
        }
    }

    void testEncouragementDelay()
    {
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastEncouragement"));
        }

        {
            Provider p;
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(1);
            p.setApplicationStartsUntilEncouragement(0);
            p.setApplicationUsageTimeUntilEncouragement(0);
            QVERIFY(!spy.wait(10));
            QVERIFY(spy.wait(1200));
            QCOMPARE(spy.count(), 1);
        }
    }

    void testNoEncouragementWithAllFeedbackEnabled()
    {
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastEncouragement"));
        }

        {
            Provider p;
            p.addDataSource(new PlatformInfoSource);
            p.setSurveyInterval(0);
            p.setTelemetryMode(Provider::BasicSystemInformation);
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setApplicationUsageTimeUntilEncouragement(0);
            QVERIFY(!spy.wait(10));
            QCOMPARE(spy.count(), 0);
        }
    }

    void testEncouragementRepetition()
    {
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.setValue(QLatin1String("LastEncouragement"), QDateTime::currentDateTime().addSecs(-24 * 60 * 60 + 1));
        }

        {
            Provider p;
            p.setSurveyInterval(-1);
            p.setTelemetryMode(Provider::NoTelemetry);
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setEncouragementInterval(1);
            QVERIFY(spy.wait(1000));
            QCOMPARE(spy.count(), 1);
        }

        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.setValue(QLatin1String("LastEncouragement"), QDateTime::currentDateTime().addSecs(-24 * 60 * 60 - 1));
        }

        {
            Provider p;
            p.setSurveyInterval(90);
            p.setTelemetryMode(Provider::BasicSystemInformation);
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setEncouragementInterval(1);
            QVERIFY(!spy.wait(10));
        }
    }

    void testGlobalEncouragementCoordination()
    {
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastEncouragement"));
        }

        {
            Provider p;
            p.setSurveyInterval(-1);
            p.setTelemetryMode(Provider::NoTelemetry);
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setEncouragementInterval(1);
            QVERIFY(spy.wait(1000));
            QCOMPARE(spy.count(), 1);
        }

        // would qualify for encouragement, but global coordination should prevent it
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastEncouragement"));
        }

        {
            Provider p;
            p.setSurveyInterval(-1);
            p.setTelemetryMode(Provider::NoTelemetry);
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setEncouragementInterval(1);
            QVERIFY(!spy.wait(100));
        }
    }

    void testMultipleProviders()
    {
        {
            Provider p0;
            p0.setTelemetryMode(Provider::BasicUsageStatistics); // triggers store, so we want to avoid that below
        }

        int c1 = -1;
        {
            Provider p1;
            auto s1 = new StartCountSource;
            s1->setTelemetryMode(Provider:: BasicUsageStatistics);
            p1.addDataSource(s1);
            c1 = s1->data().toMap().value(QLatin1String("value")).toInt();
            int c2 = -1;

            {
                Provider p2;
                auto s2 = new StartCountSource;
                s2->setTelemetryMode(Provider::BasicUsageStatistics);
                p2.addDataSource(s2);
                c2 = s2->data().toMap().value(QLatin1String("value")).toInt();
            }

            QVERIFY(c2 == c1 + 1);
        }

        Provider p3;
        auto s3 = new StartCountSource;
        s3->setTelemetryMode(Provider::BasicUsageStatistics);
        p3.addDataSource(s3);
        int c3 = s3->data().toMap().value(QLatin1String("value")).toInt();
        QVERIFY(c3 == c1 + 2);
    }

    void testProductIdChange()
    {
        {
            Provider p0;
        }

        int c1 = -1;
        {
            Provider p1;
            auto s1 = new StartCountSource;
            s1->setTelemetryMode(Provider::BasicUsageStatistics);
            p1.addDataSource(s1);
            p1.setSurveyInterval(90);
            const auto c0 = s1->data().toMap().value(QLatin1String("value")).toInt();

            p1.setProductIdentifier(QStringLiteral("org.kde.some.other.test"));
            p1.setSurveyInterval(-1);
            p1.setSurveyInterval(1);
            c1 = s1->data().toMap().value(QLatin1String("value")).toInt();
            QVERIFY(c0 != c1);
        }

        {
            Provider p2;
            auto s2 = new StartCountSource;
            s2->setTelemetryMode(Provider::BasicUsageStatistics);
            p2.addDataSource(s2);
            p2.setProductIdentifier(QStringLiteral("org.kde.some.other.test"));
            QCOMPARE(p2.surveyInterval(), 1);
            const auto c2 = s2->data().toMap().value(QLatin1String("value")).toInt();
            QCOMPARE(c1 + 1, c2);
        }
    }

    void testGlobalKillSwitch()
    {
        {
            Provider p1;
            QVERIFY(p1.isEnabled());

            p1.setEnabled(false);
            QVERIFY(!p1.isEnabled());
        }

        Provider p2;
        QVERIFY(!p2.isEnabled());

        // check encouragements are disabled
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.providertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastEncouragement"));
        }

        {
            Provider p;
            p.setSurveyInterval(-1);
            p.setTelemetryMode(Provider::NoTelemetry);
            QSignalSpy spy(&p, SIGNAL(showEncouragementMessage()));
            QVERIFY(spy.isValid());
            p.setEncouragementDelay(0);
            p.setApplicationStartsUntilEncouragement(0);
            p.setEncouragementInterval(1);
            QVERIFY(!spy.wait(100));
        }
    }

    void testDataSourceLookup()
    {
        Provider p;

        auto screenInfoDataSource = new ScreenInfoSource;
        auto screenInfoDataSourceId = screenInfoDataSource->id();

        auto platformInfoDataSource = new PlatformInfoSource;
        auto platformInfoDataSourceId = platformInfoDataSource->id();

        p.addDataSource(screenInfoDataSource);
        p.addDataSource(platformInfoDataSource);

        QCOMPARE(screenInfoDataSource, p.dataSource(screenInfoDataSourceId));
        QCOMPARE(platformInfoDataSource, p.dataSource(platformInfoDataSourceId));

        QVERIFY(!p.dataSource(QStringLiteral("SomeInvalidId")));
    }
};

QTEST_GUILESS_MAIN(ProviderTest)

#include "providertest.moc"
