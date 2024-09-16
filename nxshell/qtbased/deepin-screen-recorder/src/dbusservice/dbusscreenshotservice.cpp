// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusscreenshotservice.h"
#include "utils/eventlogutils.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QDebug>

/*
 * Implementation of adaptor class DBusScreenshotService
 */

DBusScreenshotService::DBusScreenshotService(Screenshot *parent)
    : QDBusAbstractAdaptor(parent)
    , m_singleInstance(false)
{
    // constructor
    Q_UNUSED(parent);
    setAutoRelaySignals(true);
}

DBusScreenshotService::~DBusScreenshotService()
{
    // destructor
}

void DBusScreenshotService::setSingleInstance(bool instance)
{
    m_singleInstance = instance;
}

void DBusScreenshotService::StartScreenshot()
{
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B6"}
    };
    EventLogUtils::get().writeLogs(obj);
    qDebug() << "DBus screenshot service! start screenshot";
    if (!m_singleInstance)
        parent()->startScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::DelayScreenshot(qlonglong in0)
{
    qDebug() << "DBus screenshot service! delay screenshot";
    // handle method call com.deepin.Screenshot.DelayScreenshot
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B2"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->delayScreenshot(in0);
    m_singleInstance = true;
}

void DBusScreenshotService::NoNotifyScreenshot()
{
    qDebug() << "DBus screenshot service! nonofiy screenshot";
    // handle method call com.deepin.Screenshot.NoNotify
    if (!m_singleInstance)
        parent()->noNotifyScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::OcrScreenshot()
{
    qDebug() << "DBus screenshot service! OcrScreenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B4"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->OcrScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::ScrollScreenshot()
{
    qDebug() << "DBus screenshot service! ScrollScreenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B5"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->ScrollScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::TopWindowScreenshot()
{
    qDebug() << "DBus screenshot service! topWindow screenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B3"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->topWindowScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::FullscreenScreenshot()
{
    qDebug() << "DBus screenshot service! Fullscreen screenshot";
    // handle method call com.deepin.Screenshot.Fullscreenshot
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B1"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->fullscreenScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::SavePathScreenshot(const QString &in0)
{
    qDebug() << "DBus screenshot service! SavePath screenshot";
    // handle method call com.deepin.Screenshot.SavePath
    if (!m_singleInstance)
        parent()->savePathScreenshot(in0);
    m_singleInstance = true;
}
void DBusScreenshotService::StartScreenshotFor3rd(const QString &in0)
{
    qDebug() << "DBus screenshot service! startScreenshotFor3rd";
    if (!m_singleInstance)
        parent()->startScreenshotFor3rd(in0);
    m_singleInstance = true;
}
