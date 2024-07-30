/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <provider.h>
#include <surveyinfo.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUuid>

using namespace KUserFeedback;

class SurveyProviderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<KUserFeedback::SurveyInfo>();

        QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QCoreApplication::setApplicationName(QStringLiteral("surveyprovidertest"));
        QStandardPaths::setTestModeEnabled(true);
    }

    void testSurveySelect()
    {
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.surveyprovidertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastSurvey"));
            s.remove(QLatin1String("CompletedSurveys"));
        }
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastSurvey"));
        }

        SurveyInfo s1;
        s1.setUuid(QUuid::createUuid());
        s1.setUrl(QUrl(QStringLiteral("https://www.kde.org")));
        QVERIFY(s1.isValid());

        Provider p;
        p.setSurveyInterval(90);
        QSignalSpy spy(&p, SIGNAL(surveyAvailable(KUserFeedback::SurveyInfo)));
        QVERIFY(spy.isValid());

        // global kill switch prevents survey
        bool rv = false;
        p.setEnabled(false);
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s1));
        QVERIFY(!rv);
        QVERIFY(spy.isEmpty());

        p.setEnabled(true);
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s1));
        QVERIFY(rv);
        QCOMPARE(spy.size(), 1);

        // again, not completed yet
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s1));
        QVERIFY(rv);
        QCOMPARE(spy.size(), 2);
        spy.clear();

        // survey completed, should no longer be accepted
        p.surveyCompleted(s1);
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s1));
        QVERIFY(!rv);
        QVERIFY(spy.isEmpty());

        SurveyInfo s2;
        s2.setUuid(QUuid::createUuid());
        s2.setUrl(QUrl(QStringLiteral("https://www.kde.org/survey2")));
        QVERIFY(s2.isValid());

        // next survey, but interval hasn't passed yet
        p.setSurveyInterval(90);
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s2));
        QVERIFY(!rv);
        QVERIFY(spy.isEmpty());

        // survey interval passed
        p.setSurveyInterval(0);
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s2));
        QVERIFY(rv);

        // global survey coordination prevents next survey
        p.setSurveyInterval(90);
        {
            QSettings s(QCoreApplication::organizationName(), QStringLiteral("UserFeedback.org.kde.surveyprovidertest"));;
            s.beginGroup(QLatin1String("UserFeedback"));
            s.remove(QLatin1String("LastSurvey"));
            s.remove(QLatin1String("CompletedSurveys"));
        }
        QMetaObject::invokeMethod(&p, "selectSurvey", Q_RETURN_ARG(bool, rv), Q_ARG(KUserFeedback::SurveyInfo, s1));
        QVERIFY(!rv);
    }
};

QTEST_GUILESS_MAIN(SurveyProviderTest)

#include "surveyprovidertest.moc"
