// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDesktopWidget>
#include "recordtimeplugin.h"

RecordTimePlugin::RecordTimePlugin(QObject *parent)
    : QObject(parent)
    , m_bshow(false)
{
    m_timer = nullptr;
    m_checkTimer = nullptr;
}

const QString RecordTimePlugin::pluginName() const
{
    return QString("deepin-screen-recorder-plugin");
}

const QString RecordTimePlugin::pluginDisplayName() const
{
    return QString("deepin-screen-recorder");
}

void RecordTimePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;
    m_dBusService = new DBusService(this);
    connect(m_dBusService, SIGNAL(start()), this, SLOT(onStart()));
    connect(m_dBusService, SIGNAL(stop()), this, SLOT(onStop()));
    connect(m_dBusService, SIGNAL(recording()), this, SLOT(onRecording()));
    connect(m_dBusService, SIGNAL(pause()), this, SLOT(onPause()));
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ScreenRecorder.time")
            && sessionBus.registerObject("/com/deepin/ScreenRecorder/time", this, QDBusConnection::ExportAdaptors)) {
        qDebug() << "dbus service registration failed!";
    }
    //test
    //onStart();
}

bool RecordTimePlugin::pluginIsDisable()
{
    return m_proxyInter->getValue(this, "disabled", true).toBool();
}

void RecordTimePlugin::pluginStateSwitched()
{
    const bool disabledNew = !pluginIsDisable();
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

QWidget *RecordTimePlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_timeWidget;
}

void RecordTimePlugin::clear()
{
    if (nullptr != m_timer)
    {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (nullptr != m_timeWidget) {
        m_timeWidget->deleteLater();
//        delete m_timeWidget;
        m_timeWidget = nullptr;
    }
    if (nullptr != m_checkTimer) {
        m_checkTimer->stop();
        m_checkTimer->deleteLater();
        m_checkTimer = nullptr;
    }
}

void RecordTimePlugin::onStart()
{
    m_timer = new QTimer(this);
    m_timeWidget = new TimeWidget();
    m_checkTimer = nullptr;
    m_timer->start(600);
    connect(m_timer, &QTimer::timeout, this, &RecordTimePlugin::refresh);

    if (m_timeWidget->enabled()) {
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
        m_bshow = true;
        m_timeWidget->start();
    }
}

void RecordTimePlugin::onStop()
{
    if (m_timeWidget->enabled()) {
        m_proxyInter->itemRemoved(this, pluginName());
        m_bshow = false;
        if (nullptr != m_checkTimer) {
            m_checkTimer->stop();
            m_checkTimer->deleteLater();
            m_checkTimer = nullptr;
        }
        m_count = 0;
        m_nextCount = 0;
        //m_timeWidget->stop();
        clear();
    }
}

//当托盘插件开始闪烁计数时才会执行
void RecordTimePlugin::onRecording()
{
    // 录屏过程中，killall dde-dock，恢复时重新初始化
    if (m_timeWidget == nullptr) {
        onStart();
    }
    if (m_timeWidget->enabled() && m_bshow) {
        m_nextCount++;
        if (1 == m_nextCount) {
            m_checkTimer = new QTimer();
            connect(m_checkTimer, &QTimer::timeout, this, [ = ] {
                //说明录屏还在进行中
                if (m_count < m_nextCount)
                {
                    m_count = m_nextCount;
                }
                //说明录屏已经停止了
                else
                {
                    onStop();
                }
            });
            m_checkTimer->start(2000);
        }
    }
}

void RecordTimePlugin::onPause()
{
    if (m_timeWidget->enabled() && m_bshow) {
        m_timeWidget->stop();
    }
}

void RecordTimePlugin::refresh()
{
    QSize size = m_timeWidget->sizeHint();
//    qInfo() << position() <<  " refresh >>>>>>>>>> size: " << size
//            << " , m_timeWidget->width(): " << m_timeWidget->width()
//            << " , m_timeWidget->height(): " << m_timeWidget->height()
//            << " , m_bshow: " << m_bshow;
    if (size.width() > m_timeWidget->width()
            && 1 != position()
            && 3 != position()
            && m_bshow) {
        qInfo() << "========= 重新加载插件" ;
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
    }
}

RecordTimePlugin::~RecordTimePlugin()
{

}






