/*
    This file is part of libkdbus

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QProcess>
#include <QTimer>

#include <kdbusservice.h>

#include <stdio.h>

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject(KDBusService *service)
        : m_proc(nullptr)
        , m_callCount(0)
        , m_service(service)
    {
    }

    ~TestObject() override
    {
        if (m_proc) {
            m_proc->waitForFinished();
        }
    }

    int callCount() const
    {
        return m_callCount;
    }

    void slotActivateRequested(const QStringList &args, const QString &workingDirectory)
    {
        Q_UNUSED(workingDirectory);
        qDebug() << "Application executed with args" << args;

        ++m_callCount;

        if (m_callCount == 1) {
            Q_ASSERT(args.count() == 1);
            Q_ASSERT(args.at(0) == QLatin1String("dummy call"));
        } else if (m_callCount == 2) {
            Q_ASSERT(args.count() == 2);
            Q_ASSERT(args.at(1) == QLatin1String("bad call"));
            m_service->setExitValue(4);
        } else if (m_callCount == 3) {
            Q_ASSERT(args.count() == 3);
            Q_ASSERT(args.at(1) == QLatin1String("real call"));
            Q_ASSERT(args.at(2) == QLatin1String("second arg"));
            // OK, all done, quit
            QCoreApplication::instance()->quit();
        }
    }

public Q_SLOTS:
    void firstCall()
    {
        QStringList args;
        args << QStringLiteral("bad call");
        executeNewChild(args);
    }

private Q_SLOTS:
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        Q_UNUSED(exitStatus)
        qDebug() << "Process exited with code" << exitCode;
        m_proc = nullptr;
        if (m_callCount == 2) {
            Q_ASSERT(exitCode == 4);
            secondCall();
        }
    }

    void secondCall()
    {
        QStringList args;
        args << QStringLiteral("real call") << QStringLiteral("second arg");
        executeNewChild(args);
    }

private:
    void executeNewChild(const QStringList &args)
    {
        // Duplicated from kglobalsettingstest.cpp - make a shared helper method?
        m_proc = new QProcess(this);
        connect(m_proc, &QProcess::finished, this, &TestObject::slotProcessFinished);
        QString appName = QStringLiteral("kdbusservicetest");
#ifdef Q_OS_WIN
        appName += ".exe";
#else
        if (QFile::exists(appName + ".shell")) {
            appName = "./" + appName + ".shell";
        } else {
            Q_ASSERT(QFile::exists(appName));
            appName = "./" + appName;
        }
#endif
        qDebug() << "about to run" << appName << args;
        m_proc->start(appName, args);
    }

    QProcess *m_proc;
    int m_callCount;
    KDBusService *m_service;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("kdbusservicetest"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    KDBusService service(KDBusService::Unique);
    TestObject testObject(&service);
    QObject::connect(&service, &KDBusService::activateRequested, &testObject, &TestObject::slotActivateRequested);

    // Testcase for the problem coming from the old fork-on-startup solution:
    // the "Activate" D-Bus call would time out if the app took too much time
    // to be ready.
    // printf("Sleeping.\n");
    // sleep(200);
    QStringList args;
    args << QStringLiteral("dummy call");

    auto activateSignal = [&service, &args]() {
        service.activateRequested(args, QDir::currentPath());
    };
    QMetaObject::invokeMethod(&service, activateSignal, Qt::QueuedConnection);
    QTimer::singleShot(400, &testObject, &TestObject::firstCall);
    qDebug() << "Running.";
    a.exec();
    qDebug() << "Terminating.";

    Q_ASSERT(testObject.callCount() == 3);
    const bool ok = testObject.callCount() == 3;

    return ok ? 0 : 1;
}

#include "kdbusservicetest.moc"
