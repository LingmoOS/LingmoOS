/*
    SPDX-FileCopyrightText: 2019 Jonathan Riddell <jr@jriddell.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#define MYPASSWORD "ilovekde"
#define ROOTPASSWORD "ilovekde"
#include "config-kdesutest.h"

#include <QObject>
#include <QString>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include "suprocess.h"

namespace KDESu
{
class KdeSuTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void editConfig(QString command, QString commandPath)
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group(config, QStringLiteral("super-user-command"));
        group.writeEntry("super-user-command", command);
        QString kdesuStubPath = QString::fromLocal8Bit(CMAKE_RUNTIME_OUTPUT_DIRECTORY) + QString::fromLocal8Bit("/kdesu_stub");
        group.writeEntry("kdesu_stub_path", kdesuStubPath);
        group.writeEntry("command", commandPath);
    }

    void sudoGoodPassword()
    {
        editConfig(QString::fromLocal8Bit("sudo"), QString::fromLocal8Bit(CMAKE_HOME_DIRECTORY) + QString::fromLocal8Bit("/autotests/sudo"));

        KDESu::SuProcess suProcess("root", "ls");
        QString suapp = suProcess.superUserCommand();
        QVERIFY(suapp == QLatin1String("sudo"));
        int result = suProcess.exec(MYPASSWORD, 0);
        QVERIFY(result == 0);
    }

    void sudoBadPassword()
    {
        editConfig(QString::fromLocal8Bit("sudo"), QString::fromLocal8Bit(CMAKE_HOME_DIRECTORY) + QString::fromLocal8Bit("/autotests/sudo"));

        KDESu::SuProcess suProcess("root", "ls");
        QString suapp = suProcess.superUserCommand();
        QVERIFY(suapp == QLatin1String("sudo"));
        int result2 = suProcess.exec("broken", 0);
        QVERIFY(result2 == KDESu::SuProcess::SuIncorrectPassword);
    }

    void doasBadPassword()
    {
        editConfig(QString::fromLocal8Bit("doas"), QString::fromLocal8Bit(CMAKE_HOME_DIRECTORY) + QString::fromLocal8Bit("/autotests/sudo"));

        KDESu::SuProcess suProcess("root", "ls");
        QString suapp = suProcess.superUserCommand();
        QVERIFY(suapp == QLatin1String("doas"));
        int result2 = suProcess.exec("broken", 0);
        QVERIFY(result2 == KDESu::SuProcess::SuIncorrectPassword);
    }

    void doasGoodPassword()
    {
        editConfig(QString::fromLocal8Bit("doas"), QString::fromLocal8Bit(CMAKE_HOME_DIRECTORY) + QString::fromLocal8Bit("/autotests/sudo"));

        KDESu::SuProcess suProcess("root", "ls");
        QString suapp = suProcess.superUserCommand();
        QVERIFY(suapp == QLatin1String("doas"));
        int result = suProcess.exec(MYPASSWORD, 0);
        QVERIFY(result == 0);
    }

    void suGoodPassword()
    {
        editConfig(QString::fromLocal8Bit("su"), QString::fromLocal8Bit(CMAKE_HOME_DIRECTORY) + QString::fromLocal8Bit("/autotests/su"));

        KDESu::SuProcess suProcess("root", "ls");
        QString suapp = suProcess.superUserCommand();
        QVERIFY(suapp == QLatin1String("su"));
        int result2 = suProcess.exec(ROOTPASSWORD, 0);
        QVERIFY(result2 == 0);
    }

    void suBadPassword()
    {
        editConfig(QString::fromLocal8Bit("su"), QString::fromLocal8Bit(CMAKE_HOME_DIRECTORY) + QString::fromLocal8Bit("/autotests/su"));

        KDESu::SuProcess suProcess("root", "ls");
        QString suapp = suProcess.superUserCommand();
        QVERIFY(suapp == QLatin1String("su"));
        int result2 = suProcess.exec("broken", 0);
        QVERIFY(result2 == KDESu::SuProcess::SuIncorrectPassword);
    }
};
}

#include <kdesutest.moc>
QTEST_MAIN(KDESu::KdeSuTest)
