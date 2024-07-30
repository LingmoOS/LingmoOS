/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <console/rest/serverinfo.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>

using namespace KUserFeedback;
using namespace KUserFeedback::Console;

class ServerInfoTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
        QStandardPaths::setTestModeEnabled(true);
    }

    void testServerInfoLoadStore()
    {
        ServerInfo::remove(QStringLiteral("UnitTestServer"));

        {
            const auto myInfo = ServerInfo::load(QStringLiteral("UnitTestServer"));
            QVERIFY(!myInfo.isValid());
            QVERIFY(myInfo.name().isEmpty());
            QVERIFY(myInfo.userName().isEmpty());
            QVERIFY(myInfo.password().isEmpty());
            QVERIFY(myInfo.url().isEmpty());
        }
        QVERIFY(!ServerInfo::allServerInfoNames().contains(QStringLiteral("UnitTestServer")));

        {
            ServerInfo myInfo;
            myInfo.setName(QStringLiteral("UnitTestServer"));
            myInfo.setUrl(QUrl(QStringLiteral("https://www.kde.org/")));
            myInfo.setUserName(QStringLiteral("me"));
            myInfo.setPassword(QStringLiteral("myPassword"));
            QVERIFY(myInfo.isValid());
            myInfo.save();
        }
        QVERIFY(ServerInfo::allServerInfoNames().contains(QStringLiteral("UnitTestServer")));

        {
            const auto myInfo = ServerInfo::load(QStringLiteral("UnitTestServer"));
            QVERIFY(myInfo.isValid());
            QCOMPARE(myInfo.name(), QLatin1String("UnitTestServer"));
            QCOMPARE(myInfo.userName(), QLatin1String("me"));
            QCOMPARE(myInfo.password(), QLatin1String("myPassword"));
            QCOMPARE(myInfo.url(), QUrl(QStringLiteral("https://www.kde.org/")));
        }

        ServerInfo::remove(QStringLiteral("UnitTestServer"));
    }
};

QTEST_MAIN(ServerInfoTest)

#include "serverinfotest.moc"
