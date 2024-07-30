/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * SPDX-FileCopyrightText: 2000-2002 David Faure <faure@kde.org>
 * SPDX-FileCopyrightText: 2000-2002 Waldo Bastian <bastian@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *****************************************************************/

// Let's crash.
#include <KAboutData>
#include <KCrash>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QtConcurrentMap>
#include <assert.h>

enum CrashType {
    Crash,
    Malloc,
    Div0,
    Assert,
    QAssert,
    Threads,
    FatalErrorMessage,
};

struct SomeStruct {
    int foo()
    {
        return ret;
    }
    int ret;
};

void do_crash()
{
    SomeStruct *obj = nullptr;
    int ret = obj->foo();
    printf("result = %d\n", ret);
}

void do_malloc()
{
    delete (char *)0xdead;
}

void do_div0()
{
    volatile int a = 99;
    volatile int b = 10;
    volatile int c = a / (b - 10);
    printf("result = %d\n", c);
}

void do_assert()
{
    assert(false);
}

void do_qassert()
{
    Q_ASSERT(false);
}

void map_function(const QString &s)
{
    while (s != QLatin1String("thread 4")) { }
    do_crash();
}

void do_threads()
{
    QStringList foo;
    foo << QStringLiteral("thread 1") << QStringLiteral("thread 2") << QStringLiteral("thread 3") << QStringLiteral("thread 4") << QStringLiteral("thread 5");
    QThreadPool::globalInstance()->setMaxThreadCount(5);
    QtConcurrent::blockingMap(foo, map_function);
}

void do_fatalErrorMessage()
{
    KCrash::setErrorMessage(QStringLiteral("So long, my friends..."));
    qFatal("So long!\n");
}

void level4(int t)
{
    if (t == Malloc)
        do_malloc();
    else if (t == Div0)
        do_div0();
    else if (t == Assert)
        do_assert();
    else if (t == QAssert)
        do_qassert();
    else if (t == Threads)
        do_threads();
    else if (t == FatalErrorMessage)
        do_fatalErrorMessage();
    else
        do_crash();
}

void level3(int t)
{
    level4(t);
}

void level2(int t)
{
    level3(t);
}

void level1(int t)
{
    level2(t);
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("crashtest"),
                         QStringLiteral("Crash Test for DrKonqi"),
                         QStringLiteral("1.1"),
                         QStringLiteral("Crash Test for DrKonqi"),
                         KAboutLicense::GPL,
                         QStringLiteral("(c) 2000-2002 David Faure, Waldo Bastian"));

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringLiteral("autorestart"), QStringLiteral("Automatically restart")));
    parser.addOption(QCommandLineOption(QStringLiteral("kdeinit"), QStringLiteral("Start DrKonqi using kdeinit")));
    parser.addPositionalArgument(QStringLiteral("type"), QStringLiteral("Type of crash."), QStringLiteral("crash|malloc|div0|assert|threads|fatal"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // Start drkonqi directly by default so that drkonqi's output goes to the console.
    KCrash::CrashFlags flags = KCrash::AlwaysDirectly;
    // This can be disabled to be able to test kcrash's real default behavior.
    if (parser.isSet(QStringLiteral("kdeinit")))
        flags &= ~KCrash::AlwaysDirectly;
    if (parser.isSet(QStringLiteral("autorestart")))
        flags |= KCrash::AutoRestart;
    KCrash::setFlags(flags);

    const QByteArray type = parser.positionalArguments().isEmpty() ? QByteArray() : parser.positionalArguments().constFirst().toUtf8();
    int crashtype = Crash;
    if (type == "malloc") {
        crashtype = Malloc;
    } else if (type == "div0") {
        crashtype = Div0;
    } else if (type == "assert") {
        crashtype = Assert;
    } else if (type == "qassert") {
        crashtype = QAssert;
    } else if (type == "threads") {
        crashtype = Threads;
    } else if (type == "fatal") {
        crashtype = FatalErrorMessage;
    }
    level1(crashtype);
    return app.exec();
}
