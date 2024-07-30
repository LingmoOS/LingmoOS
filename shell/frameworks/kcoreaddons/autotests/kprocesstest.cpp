/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kprocesstest_helper.h"
#include <QFile>
#include <QObject>
#include <QStandardPaths>
#include <QTest>
#include <kprocess.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

class KProcessTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_channels();
    void test_setShellCommand();
    void test_inheritance();
};

// IOCCC nomination pending

static QString callHelper(KProcess::OutputChannelMode how)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);

    QString helper = QCoreApplication::applicationDirPath() + QStringLiteral("/kprocesstest_helper");
#ifdef Q_OS_WIN
    helper += QStringLiteral(".exe");
#endif

    Q_ASSERT(QFile::exists(helper));
    p.start(helper, QStringList() << QString::number(how) << QStringLiteral("--nocrashhandler"));
    p.waitForFinished();
    return QString::fromLatin1(p.readAllStandardOutput());
}

#define EO EOUT "\n"
#define EE EERR "\n"
#define TESTCHAN(me, ms, pout, rout, rerr)                                                                                                                     \
    e = QStringLiteral("mode: " ms "\n" POUT pout ROUT rout RERR rerr);                                                                                        \
    a = QStringLiteral("mode: " ms "\n") + callHelper(KProcess::me);                                                                                           \
    QCOMPARE(a, e)

void KProcessTest::test_channels()
{
#ifdef Q_OS_UNIX
    QString e;
    QString a;
    TESTCHAN(SeparateChannels, "separate", "", EO, EE);
    TESTCHAN(ForwardedChannels, "forwarded", EO EE, "", "");
    TESTCHAN(OnlyStderrChannel, "forwarded stdout", EO, "", EE);
    TESTCHAN(OnlyStdoutChannel, "forwarded stderr", EE, EO, "");
    TESTCHAN(MergedChannels, "merged", "", EO EE, "");
#else
    Q_UNUSED(callHelper);
    QSKIP("This test needs a UNIX system");
#endif
}

void KProcessTest::test_setShellCommand()
{
    // Condition copied from kprocess.cpp
#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)
    QSKIP("This test needs a free UNIX system");
#else
    KProcess p;

    p.setShellCommand(QStringLiteral("cat"));
    QCOMPARE(p.program().count(), 1);
    QCOMPARE(p.program().at(0), QStandardPaths::findExecutable(QStringLiteral("cat")));
    QVERIFY(p.program().at(0).endsWith(QLatin1String("/cat")));
    p.setShellCommand(QStringLiteral("true || false"));
    QCOMPARE(p.program(), QStringList() << QStringLiteral("/bin/sh") << QStringLiteral("-c") << QString::fromLatin1("true || false"));
#endif
}

void KProcessTest::test_inheritance()
{
    KProcess kproc;
    QProcess *qproc = &kproc;
    const QString program = QStringLiteral("foobar");
    const QStringList arguments{QStringLiteral("meow")};

    kproc.setProgram(program, arguments);
    QCOMPARE(qproc->program(), program);
    QCOMPARE(qproc->arguments(), arguments);
    kproc.clearProgram();
    QCOMPARE(qproc->program(), QString());
    QCOMPARE(qproc->arguments(), QStringList());

    kproc << program << arguments;
    QCOMPARE(qproc->program(), program);
    QCOMPARE(qproc->arguments(), arguments);
    kproc.clearProgram();
    QCOMPARE(qproc->program(), QString());
    QCOMPARE(qproc->arguments(), QStringList());

#ifdef Q_OS_UNIX
    // not all distros have /bin/true, e.g. NixOS lacks it
    if (!QFile::exists(QStringLiteral("/bin/true"))) {
        QSKIP("This test needs /bin/true");
    }

    kproc.setShellCommand(QStringLiteral("/bin/true meow"));
    QCOMPARE(qproc->program(), QStringLiteral("/bin/true"));
    QCOMPARE(qproc->arguments(), arguments);
    kproc.clearProgram();
    QCOMPARE(qproc->program(), QString());
    QCOMPARE(qproc->arguments(), QStringList());
#endif
}

QTEST_MAIN(KProcessTest)

#include "kprocesstest.moc"
