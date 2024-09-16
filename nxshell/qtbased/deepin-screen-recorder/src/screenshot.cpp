// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshot.h"
#include "dbusinterface/dbusnotify.h"
#include "utils/configsettings.h"
#include "utils.h"
#include "utils/eventlogutils.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QWindow>

//#include <dscreenwindowsutil.h>

//DWM_USE_NAMESPACE
Screenshot::Screenshot(QObject *parent)
    : QObject(parent)
{

}

void Screenshot::startScreenshot()
{
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode(m_launchMode);
    m_window.showFullScreen();
    m_window.createWinId();
    //平板模式截图录屏
    if (Utils::isTabletEnvironment) {
        if (QString("screenRecord") == m_launchMode) {
            m_window.tableRecordSet();
        } else {
            m_window.initPadShot();
        }
    }
}

void Screenshot::delayScreenshot(double num)
{
    qDebug() << "init with delay";
    QString summary = QString(tr("Screen Capture will start in %1 seconds").arg(num));
    QStringList actions = QStringList();
    QVariantMap hints;
    DBusNotify *notifyDBus = new DBusNotify(this);
    if (num >= 2) {
        notifyDBus->Notify(Utils::appName /*QCoreApplication::applicationName()*/, 0,  "deepin-screen-recorder", "",
                           summary, actions, hints, 3000);
    }

    QTimer *timerNoti = new QTimer(this);
    timerNoti->setSingleShot(true);
    timerNoti->start(int(500 * num));
    connect(timerNoti, &QTimer::timeout, this, [ = ] {
        notifyDBus->CloseNotification(0);
    });

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(int(1000 * num));
    connect(timer, &QTimer::timeout, this, [ = ] {
        m_window.initAttributes();
        m_window.initLaunchMode("screenShot");
        m_window.showFullScreen();
        m_window.initResource();
        m_window.createWinId();
    });
}

void Screenshot::fullscreenScreenshot()
{
    m_window.fullScreenshot();
}

void Screenshot::topWindowScreenshot()
{
    m_window.topWindow();
}

void Screenshot::noNotifyScreenshot()
{
    m_window.noNotify();
}

void Screenshot::OcrScreenshot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode("screenOcr");
    m_window.showFullScreen();
    m_window.createWinId();
#endif
}

void Screenshot::ScrollScreenshot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    //2d模式不支持滚动截图
    qDebug() << "whether to trun on window effects? " << (DWindowManagerHelper::instance()->hasComposite() ? "yes" : "no");
    if (DWindowManagerHelper::instance()->hasComposite()) {
        qDebug() << "start scroll shot !";
        m_window.initAttributes();
        m_window.initResource();
        m_window.initLaunchMode("screenScroll");
        m_window.showFullScreen();
        m_window.createWinId();
    } else {
        qDebug() << "scroll shot exit !";
        qApp->quit();
        if (Utils::isWaylandMode) {
            _Exit(0);
        }
    }
#else
    qApp->quit();
    if (Utils::isWaylandMode) {
        _Exit(0);
    }
#endif
}

void Screenshot::savePathScreenshot(const QString &path)
{
    m_window.savePath(path);
}

void Screenshot::startScreenshotFor3rd(const QString &path)
{
    Utils::is3rdInterfaceStart = true;
    m_window.startScreenshotFor3rd(path);
}

void Screenshot::initLaunchMode(const QString &launchmode)
{
    m_launchMode = launchmode;
}

void Screenshot::stopRecord()
{
    m_window.stopRecord();
}

QString Screenshot::getRecorderNormalIcon()
{
    return RecorderTablet::getRecorderNormalIcon();
}

Screenshot::~Screenshot()
{
}
