/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTest>

class KCrashTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // Don't bring up drkonqi
        qputenv("KDE_DEBUG", "1");
        // change to the bin dir
        QDir::setCurrent(QCoreApplication::applicationDirPath());
    }
    void testAutoRestart();
    void testAutoRestartDirectly();
    void testEmergencySave();
};

static const char s_logFileName[] = "kcrashtest_log";

static QByteArray readLogFile()
{
    QFile logFile(QFile::encodeName(s_logFileName));
    if (!logFile.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return logFile.readAll();
}

static void startCrasher(const QByteArray &flag, const QByteArray &expectedOutput)
{
    QFile::remove(QFile::encodeName(s_logFileName));

    QProcess proc;
    QString processName;
#ifdef Q_OS_WIN
    QVERIFY(QFile::exists("./test_crasher.exe"));
    processName = "test_crasher.exe";
#else
    QVERIFY(QFile::exists("./test_crasher"));
    processName = QStringLiteral("./test_crasher");
#endif
    // qDebug() << proc.args();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("ASAN_OPTIONS"), QStringLiteral("handle_segv=0,poison_heap=0")); // Disable ASAN
    proc.setProcessEnvironment(env);
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.start(processName, QStringList() << flag);
    bool ok = proc.waitForFinished();
    QVERIFY(ok);

    QByteArray logData;
    for (int i = 0; i < 50; ++i) {
        logData = readLogFile();
        if (logData == expectedOutput) {
            return;
        }
        QTest::qSleep(100);
    }
    qDebug() << proc.readAllStandardError();
    QCOMPARE(QString(logData), QString(expectedOutput));
}

void KCrashTest::testAutoRestart() // use kdeinit if possible, otherwise directly (ex: on CI)
{
    startCrasher("AR", "starting AR\nautorestarted AR\n");
}

void KCrashTest::testAutoRestartDirectly() // test directly (so a developer can test the CI case)
{
    startCrasher("ARD", "starting ARD\nautorestarted ARD\n");
}

void KCrashTest::testEmergencySave()
{
    startCrasher("ES", "starting ES\nsaveFunction called\n");
}

QTEST_MAIN(KCrashTest)

#include "kcrashtest.moc"
