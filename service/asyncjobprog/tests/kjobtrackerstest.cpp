/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kjobtrackerstest.h"
#include "kdialogjobuidelegate.h"

#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QStatusBar>

#include <kjobwidgets.h>
#include <kstatusbarjobtracker.h>
#include <kuiserverjobtracker.h>
#include <kwidgetjobtracker.h>

KTestJob::KTestJob(int numberOfDirs)
    : KJob()
    , m_numberOfDirs(numberOfDirs)
    , m_currentSpeed(1000)
    , m_state(Stopped)
{
    setCapabilities(KJob::Killable | KJob::Suspendable);
}

KTestJob::~KTestJob()
{
}

void KTestJob::start()
{
    connect(&m_timer, &QTimer::timeout, this, &KTestJob::nextStep);
    m_state = StatingDirs;
    m_timer.start(50);
    Q_EMIT description(this,
                       QStringLiteral("Copying"),
                       qMakePair(QStringLiteral("Source"), QStringLiteral("file:/src")),
                       qMakePair(QStringLiteral("Destination"), QStringLiteral("file:/dest")));
}

void KTestJob::nextStep()
{
    switch (m_state) {
    case StatingDirs:
        Q_EMIT infoMessage(this, QStringLiteral("Initial listing"));
        stateNextDir();
        break;
    case CreatingDirs:
        Q_EMIT infoMessage(this, QStringLiteral("Folder creation"));
        createNextDir();
        break;
    case CopyingFiles:
        Q_EMIT infoMessage(this, QStringLiteral("Actual file copying"));
        copyNextFile();
        break;
    case Stopped:
        qDebug() << "Do nothing, we stopped";
    }
}

void KTestJob::stateNextDir()
{
    if (totalAmount(KJob::Directories) == m_numberOfDirs) {
        m_state = CreatingDirs;
        return;
    }

    QString directory_name = QStringLiteral("dir") + QString::number(totalAmount(KJob::Directories));

    qDebug() << "Stating " << directory_name;
    setTotalAmount(KJob::Directories, totalAmount(KJob::Directories) + 1);
    setTotalAmount(KJob::Files, totalAmount(KJob::Directories) * 10);
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Files) * 1000);

    Q_EMIT warning(this, directory_name, directory_name);
    Q_EMIT description(this, QStringLiteral("Stating"), qMakePair(QStringLiteral("Stating"), QString(QStringLiteral("file:/src/") + directory_name)));
}

void KTestJob::createNextDir()
{
    if (processedAmount(KJob::Directories) == totalAmount(KJob::Directories)) {
        m_state = CopyingFiles;
        return;
    }

    QString directory_name = QStringLiteral("dir") + QString::number(processedAmount(KJob::Directories));

    qDebug() << "Creating " << directory_name;
    setProcessedAmount(KJob::Directories, processedAmount(KJob::Directories) + 1);

    Q_EMIT description(this, QStringLiteral("Creating Dir"), qMakePair(QStringLiteral("Creating"), QString(QStringLiteral("file:/dest/") + directory_name)));
}

void KTestJob::copyNextFile()
{
    if (processedAmount(KJob::Files) == totalAmount(KJob::Files)) {
        m_state = Stopped;
        m_timer.stop();
        emitResult();
        return;
    }

    QString file_name =
        QLatin1String("dir%1/file%2").arg(QString::number(processedAmount(KJob::Files) / 10), QString::number(processedAmount(KJob::Files) % 10));

    qDebug() << "Copying " << file_name;
    setProcessedAmount(KJob::Files, processedAmount(KJob::Files) + 1);
    setProcessedAmount(KJob::Bytes, processedAmount(KJob::Bytes) + 1000);

    Q_EMIT description(this,
                       QStringLiteral("Copying"),
                       qMakePair(QStringLiteral("Source"), QString(QStringLiteral("file:/src/") + file_name)),
                       qMakePair(QStringLiteral("Destination"), QString(QStringLiteral("file:/dest/") + file_name)));

    emitSpeed(m_currentSpeed);
}

bool KTestJob::doSuspend()
{
    m_timer.stop();
    return true;
}

bool KTestJob::doResume()
{
    m_timer.start(50);
    return true;
}

bool KTestJob::doKill()
{
    m_timer.stop();
    m_state = Stopped;
    return true;
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kjobtrackerstest"));

    QApplication app(argc, argv);

    KTestJob *testJob = new KTestJob(10 /* 100000 bytes to process */);

    KWidgetJobTracker *tracker1 = new KWidgetJobTracker();
    tracker1->registerJob(testJob);

    QMainWindow *main = new QMainWindow;
    main->setWindowTitle(QStringLiteral("Mainwindow with statusbar-job-tracker"));
    main->show();

    QStatusBar *statusBar = new QStatusBar(main);
    KStatusBarJobTracker *tracker2 = new KStatusBarJobTracker(main, true);
    tracker2->registerJob(testJob);
    tracker2->setStatusBarMode(KStatusBarJobTracker::ProgressOnly);
    statusBar->addWidget(tracker2->widget(testJob));

    main->setStatusBar(statusBar);

    KUiServerJobTracker *tracker3 = new KUiServerJobTracker(main);
    tracker3->registerJob(testJob);

    KJobWidgets::setWindow(testJob, main);
    testJob->setUiDelegate(new KDialogJobUiDelegate());

    testJob->start();

    tracker1->widget(testJob)->show();
    tracker2->widget(testJob)->show();

    return app.exec();
}
