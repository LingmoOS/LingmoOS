/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "lingmo-appmotor.h"
#include "daemon.h"

#include <unistd.h>

#include <QQuickView>
#include <QtGlobal>
#include <QApplication>
#include <QDebug>

const string LingmoBooster::m_boosterType = "lingmo";

const string & LingmoBooster::boosterType() const
{
    return m_boosterType;
}

int LingmoBooster::launchProcess()
{
    Booster::setEnvironmentBeforeLaunch();

    // Ensure a NULL-terminated argv
    char ** dummyArgv = new char * [appData()->argc() + 1];
    const int argc = appData()->argc();
    for (int i = 0; i < argc; i++)
        dummyArgv[i] = strdup(appData()->argv()[i]);

    dummyArgv[argc] = NULL;

    // Exec the binary (execv returns only in case of an error).
    execv(appData()->fileName().c_str(), dummyArgv);

    // Delete dummy argv if execv failed
    for (int i = 0; i < argc; i++)
        free(dummyArgv[i]);

    delete [] dummyArgv;

    return EXIT_FAILURE;
}

void LingmoBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket,
                           int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    new QApplication(initialArgc, initialArgv);
    Booster::initialize(initialArgc, initialArgv, boosterLauncherSocket, socketFd, singleInstance, bootMode);
}

bool LingmoBooster::preload()
{
    QQuickView window;
    window.create();

    return true;
}

int main(int argc, char **argv)
{
    LingmoBooster *booster = new LingmoBooster;

    Daemon d(argc, argv);
    d.run(booster);
}

