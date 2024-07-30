/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Gregor Mi <codeminister@publicstatic.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "krecentfilesactiontest.h"

#include <QApplication>
#include <QDebug>

#include "krecentfilesaction.h"
#include "kstandardaction.h"
#include <KConfigGroup>
#include <KSharedConfig>

#include "ui_krecentfilesactiontest.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KRecentFilesActionTest mainWindow;
    mainWindow.show();
    return app.exec();
}

class KRecentFilesActionTestPrivate
{
public:
    Ui::MainWindow *uiMainWindow;
    KRecentFilesAction *recentFiles;

public:
    void notifyOutputAvailable()
    {
        uiMainWindow->labelOutputAvailable->setText(uiMainWindow->labelOutputAvailable->text() + QLatin1Char('A'));
        qDebug() << recentFiles->items();
    }

    KConfigGroup testConfigGroup()
    {
        return KConfigGroup(KSharedConfig::openConfig(), "RecentFilesActionTest");
    }
};

KRecentFilesActionTest::KRecentFilesActionTest()
    : d(new KRecentFilesActionTestPrivate)
{
    d->uiMainWindow = new Ui::MainWindow();
    d->uiMainWindow->setupUi(this);

    d->recentFiles = KStandardAction::openRecent(this, &KRecentFilesActionTest::urlSelected, this);

    connect(d->uiMainWindow->pbAddUrl, &QPushButton::clicked, this, &KRecentFilesActionTest::addUrl);
    connect(d->uiMainWindow->pbLoadEntries, &QPushButton::clicked, this, &KRecentFilesActionTest::loadEntries);
    connect(d->uiMainWindow->pbSaveEntries, &QPushButton::clicked, this, &KRecentFilesActionTest::saveEntries);

    d->uiMainWindow->menuFile->addAction(d->recentFiles);

    // loadEntries();
}

KRecentFilesActionTest::~KRecentFilesActionTest()
{
    // saveEntries();

    delete d->uiMainWindow;
    delete d;
}

void KRecentFilesActionTest::urlSelected(const QUrl &url)
{
    qDebug() << "urlSelected" << url;
    d->notifyOutputAvailable();
}

void KRecentFilesActionTest::addUrl()
{
    QString url = d->uiMainWindow->lineEditUrl->text();
    qDebug() << "addUrl" << url;

    d->recentFiles->addUrl(QUrl(url));

    d->notifyOutputAvailable();
    d->uiMainWindow->lineEditUrl->setText(url + QLatin1Char('a'));
}

void KRecentFilesActionTest::loadEntries()
{
    d->notifyOutputAvailable();
    qDebug() << "recentFiles->loadEntries()";
    d->recentFiles->loadEntries(d->testConfigGroup());
    d->notifyOutputAvailable();
}

void KRecentFilesActionTest::saveEntries()
{
    qDebug() << "recentFiles->saveEntries()";
    d->recentFiles->saveEntries(d->testConfigGroup());
}

#include "moc_krecentfilesactiontest.cpp"
