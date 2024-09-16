// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DLog>
#include <QCoreApplication>
#include <QTimer>

DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifdef Q_OS_LINUX
    DLogManager::registerJournalAppender();
#endif
    DLogManager::registerConsoleAppender();
    dDebug() << "Anything that can possibly go wrong, will go wrong. ";
    qWarning() << "FBI Warning: Code smells.";
    qInfo() << "Why dark mode? Because light attracts bugs";
    qCritical() << "You Should Never Run `sudo rm -rf /`";

    {
        dDebugTime("Test the running time of a code block");
        QTimer timer;
        timer.setInterval(500);
        QEventLoop loop;
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();
    }

    return app.exec();
}
