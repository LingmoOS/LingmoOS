/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "test_kconf_update.h"

#include <memory>

// Qt
#include <QDir>
#include <QFile>
#include <QProcess>
#include <qglobal.h>
#include <qstandardpaths.h>
#include <qtemporaryfile.h>
#include <qtest.h>
#include <qtestcase.h>

QTEST_GUILESS_MAIN(TestKConfUpdate)

void TestKConfUpdate::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // Ensure it all works with spaces in paths (as happens more commonly on OSX where it's ~/Library/Application Support/)
    qputenv("XDG_DATA_HOME", "/tmp/a b");
}

static void writeFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qFatal("Could not write to '%s'", qPrintable(path));
    }
    file.write(content.toUtf8());
}

static QString readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Could not read '%s'", qPrintable(path));
    }
    QString ret = QString::fromUtf8(file.readAll());
#ifdef Q_OS_WIN
    // KConfig always writes files with the native line ending, the test comparison uses \n
    ret.replace(QLatin1String{"\r\n"}, QLatin1String{"\n"});
#endif
    return ret;
}

static std::unique_ptr<QTemporaryFile> writeUpdFile(const QString &content)
{
    std::unique_ptr<QTemporaryFile> file{new QTemporaryFile(QDir::tempPath() + QLatin1String("/test_kconf_update_XXXXXX.upd"))};
    bool ok = file->open();
    Q_UNUSED(ok) // silence warnings
    Q_ASSERT(ok);
    file->write(content.toUtf8());
    file->flush();
    return file;
}

static void runKConfUpdate(const QString &updPath, const QString &expectedKConfOutputString)
{
    const QString kconfUpdateExecutable = QFINDTESTDATA("kconf_update");
    QVERIFY(QFile::exists(kconfUpdateExecutable));
    QProcess p;
    p.start(kconfUpdateExecutable, QStringList{QStringLiteral("--testmode"), QStringLiteral("--debug"), updPath});
    QVERIFY(p.waitForFinished());
    QCOMPARE(p.exitCode(), 0);
    if (!expectedKConfOutputString.isEmpty()) {
        const QString out = QString::fromLocal8Bit(p.readAllStandardError());
        QVERIFY2(out.contains(expectedKConfOutputString),
                 qPrintable(QLatin1String("The string \"%1\" was not contained in putput \"%2\"").arg(expectedKConfOutputString, out)));
    }
}

void TestKConfUpdate::testScript_data()
{
    QTest::addColumn<QString>("updContent");
    QTest::addColumn<QString>("updScript");
    QTest::addColumn<QString>("oldConfContent");
    QTest::addColumn<QString>("expectedNewConfContent");
    QTest::addColumn<QString>("expectedKConfOutputString");

#ifdef Q_OS_FREEBSD
    // https://stackoverflow.com/a/69955786
    const QString sedCommand = QStringLiteral("sed -i '' ");
#else
    const QString sedCommand = QStringLiteral("sed -i ");
#endif
    const QString updVersionIdPrefix = QLatin1String("Version=6\nId=%1\n").arg(QLatin1String{QTest::currentDataTag()});
    const QString confPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/testrc"};
    const QString scriptContent = sedCommand + QLatin1String("'s/firstVal=abc/firstVal=xyz/' %1").arg(confPath);

    QTest::newRow("should reject script due to version missmatch")
        << QStringLiteral("Version=5\nId=12345") << scriptContent << QString() << QString() << QStringLiteral("defined Version=5 but Version=6 was expected");

    const QString updContent = updVersionIdPrefix + QLatin1String("Script=test.sh,sh\n");
    const QString configIn = QStringLiteral("[grp]\nfirstVal=abc\nsecondVal=xyz\n");
    const QString configOut = QStringLiteral("[grp]\nfirstVal=xyz\nsecondVal=xyz\n");

    QTest::newRow("should run command and modify file") << updContent << scriptContent << configIn << configOut << QString();

    const QString argScriptContent = sedCommand + QLatin1String("\"s/secondVal=$1/secondVal=abc/\" %1").arg(confPath);
    QTest::newRow("should run command with argument and modify file") << QStringLiteral("ScriptArguments=xyz\n") + updContent << argScriptContent << configIn
                                                                      << QStringLiteral("[grp]\nfirstVal=abc\nsecondVal=abc\n") << QString();

    QTest::newRow("should run command with arguments and modify file")
        << QStringLiteral("ScriptArguments=xyz abc\n") + updContent << sedCommand + QLatin1String("\"s/secondVal=$1/secondVal=$2/\" %1").arg(confPath)
        << configIn << QStringLiteral("[grp]\nfirstVal=abc\nsecondVal=abc\n") << QString();
}

void TestKConfUpdate::testScript()
{
    if (QStandardPaths::findExecutable(QStringLiteral("sh")).isEmpty()) {
        QSKIP("Could not find sh executable, cannot run test");
        return;
    }

    QFETCH(QString, updContent);
    QFETCH(QString, updScript);
    QFETCH(QString, oldConfContent);
    QFETCH(QString, expectedNewConfContent);
    QFETCH(QString, expectedKConfOutputString);

    std::unique_ptr<QTemporaryFile> updFile(writeUpdFile(updContent));

    const QString scriptDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String{"/kconf_update"};
    QVERIFY(QDir().mkpath(scriptDir));
    const QString scriptPath = scriptDir + QLatin1String{"/test.sh"};
    writeFile(scriptPath, updScript);
    QCOMPARE(readFile(scriptPath), updScript);

    const QString confPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/testrc"};
    writeFile(confPath, oldConfContent);
    QCOMPARE(readFile(confPath), oldConfContent);

    runKConfUpdate(updFile->fileName(), expectedKConfOutputString);

    QCOMPARE(readFile(confPath), expectedNewConfContent);
}

#include "moc_test_kconf_update.cpp"
