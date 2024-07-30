/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWINDOWSYSTEM_P_H
#define KWINDOWSYSTEM_P_H

#include "netwm_def.h"
#include <QStringList>
#include <QWidgetList> //For WId
#include <kwindowsystem_export.h>

class NETWinInfo;

class KWINDOWSYSTEM_EXPORT KWindowSystemPrivate : public NET
{
public:
    virtual ~KWindowSystemPrivate();
    virtual void activateWindow(QWindow *win, long time = 0) = 0;
    virtual bool showingDesktop() = 0;
    virtual void setShowingDesktop(bool showing) = 0;
};

class KWINDOWSYSTEM_EXPORT KWindowSystemPrivateV2 : public KWindowSystemPrivate
{
public:
    virtual void requestToken(QWindow *win, uint32_t serial, const QString &app_id) = 0;
    virtual void setCurrentToken(const QString &token) = 0;
    virtual quint32 lastInputSerial(QWindow *window) = 0;
    virtual void setMainWindow(QWindow *window, const QString &handle) = 0;
    virtual void exportWindow(QWindow *window) = 0;
    virtual void unexportWindow(QWindow *window) = 0;
};

#endif
