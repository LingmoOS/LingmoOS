// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordertablet.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

const QString RecorderTablet::RESOURCES_PATH = "/usr/share/deepin-screen-recorder/tablet_resources";

RecorderTablet::RecorderTablet(QObject *parent) : QObject(parent)
{

    QDBusConnection sb = QDBusConnection::sessionBus();
    bool result;
    result = sb.registerService("com.deepin.TabletScreenRecorder");
    if (!result) {
        qWarning() << "failed to register dbus service" << sb.lastError().message();
    }

    result = sb.registerObject("/com/deepin/TabletScreenRecorder",
                               "com.deepin.TabletScreenRecorder", this,
                               QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots);
    if (!result) {
        qWarning() << "failed to register dbus object" << sb.lastError().message();
    }

}

RecorderTablet::~RecorderTablet()
{
    if (nullptr != m_statusBarTimer) {
        delete m_changeTimer;
    }
    if (nullptr != m_statusBarTimer) {
        delete m_statusBarTimer;
    }
}

void RecorderTablet::start()
{
    // 拷贝json文件到指定目录
    /*
    QString dirPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    dirPath += "/Applications/";
    QDir dir;
    if (!dir.exists(dirPath)) {
        dir.mkdir(dirPath);
    }
    QFile::copy(RESOURCES_PATH + "/recorder.json", dirPath + "recorder.json");
    */
    // 通知开始录屏
    sendRecorderState(true);


    showCountdownCounter = 3;
    m_changeTimer = new QTimer(this);
    connect(m_changeTimer, SIGNAL(timeout()), this, SLOT(update()));
    m_changeTimer->start(1000);
}

void RecorderTablet::stop()
{
    m_statusBarTimer->stop();
    // 下拉面板图标设置为非录屏状态
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "DropDownIconChange");
    msg << QString("%1/%2").arg(RESOURCES_PATH).arg("fast-icon_recording_normal.svg");
    QDBusConnection::sessionBus().send(msg);
    // 删除json文件
    // QFile::remove(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/Applications/recorder.json");

    // 通知停止录屏
    sendRecorderState(false);
}

void RecorderTablet::update()
{

    //下拉面板发送dbus信号， 倒计时图片
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "DropDownIconChange");
    msg << QString("%1/counting%2.svg").arg(RESOURCES_PATH).arg(showCountdownCounter);
    QDBusConnection::sessionBus().send(msg);

    showCountdownCounter--;
    if (showCountdownCounter <= 0) {
        m_changeTimer->stop();
        emit finished();
    }
}

void RecorderTablet::startStatusBar()
{
    // 下拉面板图标设置为录屏状态
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "DropDownIconChange");
    msg << QString("%1/%2").arg(RESOURCES_PATH).arg("fast-icon_recording_active.svg");
    QDBusConnection::sessionBus().send(msg);

    m_statusBarTimer = new QTimer(this);
    connect(m_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
    m_statusBarTimer->start(800);
}

const QString RecorderTablet::getRecorderNormalIcon()
{
    return RecorderTablet::RESOURCES_PATH + "/fast-icon_recording_normal.svg";
}

void RecorderTablet::updateStatusBar()
{
    //状态栏发送dbus信号
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "StatusBarIconChange");
    msg << QString("%1/recording%2.svg").arg(RESOURCES_PATH).arg(flashTrayIconCounter % 2);
    QDBusConnection::sessionBus().send(msg);
    flashTrayIconCounter++;
}

void RecorderTablet::sendRecorderState(const bool state)
{
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/ScreenRecorder", "com.deepin.ScreenRecorder", "RecorderState");
    msg << state;
    QDBusConnection::sessionBus().send(msg);
}
