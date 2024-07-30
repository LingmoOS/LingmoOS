/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kptyprocesstest.h"

#include <QDebug>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QThread>
#include <kptydevice.h>

void KPtyProcessTest::test_suspend_pty()
{
    KPtyProcess p;
    p.setPtyChannels(KPtyProcess::AllChannels);
    p.setProgram("/bin/sh",
                 QStringList() << "-c"
                               << "while true; do echo KPtyProcess_test; sleep 1; done");
    p.start();

    // verify that data is available to read from the pty
    QVERIFY(p.pty()->waitForReadyRead(1500));

    // suspend the pty device and read all available data from it
    p.pty()->setSuspended(true);
    QVERIFY(p.pty()->isSuspended());
    p.pty()->readAll();

    // verify that no data was read by the pty
    QVERIFY(!p.pty()->waitForReadyRead(1500));

    // allow process to write more data
    p.pty()->setSuspended(false);
    QVERIFY(!p.pty()->isSuspended());

    // verify that data is available once more
    QVERIFY(p.pty()->waitForReadyRead(2000));
    p.pty()->readAll();

    p.terminate();
    p.waitForFinished();
}

void KPtyProcessTest::test_shared_pty()
{
    // start a first process
    KPtyProcess p;
    p.setProgram("cat");
    p.setPtyChannels(KPtyProcess::AllChannels);
    p.pty()->setEcho(false);
    p.start();

    // start a second process using the first one's fd
    int fd = p.pty()->masterFd();

    KPtyProcess p2(fd);
    p2.setProgram("echo", QStringList() << "hello from me");
    p2.setPtyChannels(KPtyProcess::AllChannels);
    p2.pty()->setEcho(false);
    p2.start();

    // read the second processes greeting from the first process' pty
    for (int i = 0; i < 5; ++i) {
        QVERIFY(p.pty()->waitForReadyRead(500));
        if (p.pty()->canReadLine()) {
            break;
        }
    }
    QCOMPARE(p.pty()->readAll(), QByteArray("hello from me\r\n"));

    // write to the second process' pty
    p2.pty()->write("hello from process 2\n");
    QVERIFY(p2.pty()->waitForBytesWritten(1000));

    // read the result back from the first process' pty
    for (int i = 0; i < 5; ++i) {
        QVERIFY(p.pty()->waitForReadyRead(500));
        if (p.pty()->canReadLine()) {
            break;
        }
    }
    QCOMPARE(p.pty()->readAll(), QByteArray("hello from process 2\r\n"));

    // write to the first process' pty
    p.pty()->write("hi from process 1\n");
    QVERIFY(p.pty()->waitForBytesWritten(1000));

    // read the result back from the second process' pty
    for (int i = 0; i < 5; ++i) {
        QVERIFY(p2.pty()->waitForReadyRead(500));
        if (p2.pty()->canReadLine()) {
            break;
        }
    }
    QCOMPARE(p2.pty()->readAll(), QByteArray("hi from process 1\r\n"));

    // cleanup
    p.terminate();
    p2.terminate();
    p.waitForFinished(1000);
    p2.waitForFinished(1000);
}

void KPtyProcessTest::test_pty_basic()
{
    const QString bash = QStandardPaths::findExecutable("bash");
    if (bash.isEmpty()) {
        QSKIP("bash is not installed");
    }

#ifdef Q_OS_FREEBSD
    QSKIP("This test fails on FreeBSD for some reason (waitForReadyRead(5000) times out)");
#endif
    KPtyProcess p;
    p.setProgram(bash,
                 QStringList() << "-c"
                               << "read -s VAL; echo \"1: $VAL\"; echo \"2: $VAL\" >&2");
    p.setPtyChannels(KPtyProcess::AllChannels);
    p.pty()->setEcho(false);
    p.start();
    p.pty()->write("test\n");
    p.pty()->waitForBytesWritten(1000);
    QVERIFY(p.waitForFinished(5000));
    while (p.pty()->bytesAvailable() < 18) {
        qDebug() << p.pty()->bytesAvailable() << "bytes available, waiting";
        QVERIFY(p.pty()->waitForReadyRead(5000));
    }
    QString output = p.pty()->readAll();
    QCOMPARE(output, QLatin1String("1: test\r\n2: test\r\n"));
}

void KPtyProcessTest::slotReadyRead()
{
    delay.start(30);
}

void KPtyProcessTest::slotDoRead()
{
    while (sp.pty()->canReadLine()) {
        log.append('>').append(sp.pty()->readLine()).append("$\n");
    }
    log.append("!\n");
}

void KPtyProcessTest::slotReadEof()
{
    log.append('|').append(sp.pty()->readAll()).append("$\n");
}

void KPtyProcessTest::slotBytesWritten()
{
    log.append('<');
}

static const char *const feeds[] = {"bla\n", "foo\x04", "bar\n", "fooish\nbar\n", "\x04", nullptr};

static const char want[] =
    "<>bla\r\n$\n!\n"
    "<!\n<>foobar\r\n$\n!\n"
    "<>fooish\r\n$\n>bar\r\n$\n!\n"
    "<|$\n";

void KPtyProcessTest::slotStep()
{
    if (feeds[phase]) {
        sp.pty()->write(feeds[phase]);
        phase++;
    }
}

Q_DECLARE_METATYPE(QProcess::ExitStatus)

void KPtyProcessTest::test_pty_signals()
{
    sp.setShellCommand("cat; sleep .1");
    sp.setPtyChannels(KPtyProcess::StdinChannel | KPtyProcess::StdoutChannel);
    sp.pty()->setEcho(false);
    connect(sp.pty(), &QIODevice::readyRead, this, &KPtyProcessTest::slotReadyRead);
    connect(sp.pty(), &KPtyDevice::readEof, this, &KPtyProcessTest::slotReadEof);
    connect(sp.pty(), &QIODevice::bytesWritten, this, &KPtyProcessTest::slotBytesWritten);
    QTimer timer;
    connect(&timer, &QTimer::timeout, this, &KPtyProcessTest::slotStep);
    timer.start(200);
    connect(&delay, &QTimer::timeout, this, &KPtyProcessTest::slotDoRead);
    delay.setSingleShot(true);
    sp.start();
    sp.pty()->closeSlave();
    phase = 0;
    qRegisterMetaType<QProcess::ExitStatus>();
    QSignalSpy finishedSpy(&sp, &QProcess::finished);
    QVERIFY(finishedSpy.wait(2000));
    log.replace("<<", "<"); // It's OK if bytesWritten is emitted multiple times...
    QCOMPARE(QLatin1String(log), QLatin1String(want));
}

void KPtyProcessTest::test_ctty()
{
#ifdef Q_OS_MAC
    QSKIP("This test currently hangs on OSX");
#endif
#ifdef Q_OS_FREEBSD
    QSKIP("This test fails on FreeBSD for some reason (output is empty)");
#endif

    KPtyProcess p;
    p.setShellCommand("echo this is a test > /dev/tty");
    p.execute(1000);
    p.pty()->waitForReadyRead(1000);
    QString output = p.pty()->readAll();
    QCOMPARE(output, QLatin1String("this is a test\r\n"));
}

QTEST_MAIN(KPtyProcessTest)

#include "moc_kptyprocesstest.cpp"
