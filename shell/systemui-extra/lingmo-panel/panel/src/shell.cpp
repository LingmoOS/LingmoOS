/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *          iaom <zhangpengfei@kylinos.cn>
 */

#include "shell.h"
#include "panel.h"

#include <QScreen>
#include <QGuiApplication>
#include <config-loader.h>
#include <QDBusInterface>
#include <QDBusReply>

using namespace LingmoUIPanel;

Shell::Shell(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QScreen*>();
    m_screenManager = new ScreensManager(this);
}

void Shell::start()
{
    //在创建任务栏之前读配置文件
    initPanelConfig();

    Screen *primary = m_screenManager->primaryScreen();
    auto panel = new Panel(primary, "panel0");
    connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
    m_panels.insert(primary, panel);
    //如果设置为在所有屏幕上显示任务栏再创建副屏的任务栏，否则不创建
    if(m_showPanelOnAllScreens) {
        for(Screen * screen : m_screenManager->screens()) {
            if(screen != primary) {
                panel = new Panel(screen, QString("panel%1").arg(m_panels.size()));
                connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
                m_panels.insert(screen, panel);
            }
        }
    }

    initDbus();
    initRemoteConfig();

    connect(m_screenManager, &ScreensManager::screenAdded, this, [&](Screen *screen){
        if(!m_showPanelOnAllScreens) {
            return;
        }
        if(!m_panels.contains(screen)) {
            panel = new Panel(screen, QString("panel%1").arg(m_panels.size()));
            connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
            m_panels.insert(screen, panel);
        }
    });
    connect(m_screenManager, &ScreensManager::screenRemoved, this, [&](Screen *screen){
        if(m_panels.contains(screen)) {
            delete m_panels.take(screen);
        }
    });
    //主屏变化时，如果为只在主屏上显示任务栏，将主屏任务栏重新setscreen
    connect(m_screenManager, &ScreensManager::primaryScreenChanged, this, [&](Screen *screen){
        if(!m_showPanelOnAllScreens) {
            auto panel = m_panels.first();
            m_panels.clear();
            panel->setPanelScreen(screen);
            m_panels.insert(screen, panel);
        }
    });
}

void Shell::initRemoteConfig()
{
    m_remoteConfig = new RemoteConfig(this);
    QString display = QCoreApplication::instance()->property("display").toString();
    QUrl address(QStringLiteral("local:lingmo-panel-config-")  + QString(qgetenv("USER")) + display);
    m_qroHost.setHostUrl(address);
    qDebug() << "Init remote status object:" << m_qroHost.enableRemoting<RemoteConfigSourceAPI>(m_remoteConfig);
}

void Shell::initDbus()
{
    //dbus
    const QString statusManagerService = "com.lingmo.statusmanager.interface";
    QDBusInterface statusManagerDBus(statusManagerService, "/" , statusManagerService,
                                     QDBusConnection::sessionBus(), this);

    if (statusManagerDBus.isValid()) {
        QDBusReply<bool> message = statusManagerDBus.call("get_current_tabletmode");
        if (message.isValid()) {
            m_isTabletMode = message.value();
        }
    }

    onTabletModeChanged(m_isTabletMode);
    //平板模式切换信号
    QDBusConnection::sessionBus().connect(statusManagerService, "/", statusManagerService,
                                          "mode_change_signal", this, SLOT(onTabletModeChanged(bool)));
}

void Shell::onTabletModeChanged(bool isTabletMode)
{
    m_isTabletMode = isTabletMode;
    for (const auto &panel : m_panels) {
        panel->setVisible(!m_isTabletMode);
    }
}

void Shell::initPanelConfig()
{
    if(!m_panelConfig) {
        m_panelConfig = LingmoUIQuick::ConfigLoader::getConfig("panel", LingmoUIQuick::ConfigLoader::Local, "lingmo-panel");
    }
    if(m_panelConfig->contains("showPanelOnAllScreens")) {
        m_showPanelOnAllScreens = m_panelConfig->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
    } else {
        m_showPanelOnAllScreens = true;
        m_panelConfig->setValue(QStringLiteral("showPanelOnAllScreens"), m_showPanelOnAllScreens);
    }
    connect(m_panelConfig, &LingmoUIQuick::Config::configChanged, this, [&](QString key) {
        if(key == "showPanelOnAllScreens") {
            auto value = m_panelConfig->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
            if(value == m_showPanelOnAllScreens) {
                return;
            }
            m_showPanelOnAllScreens = value;
            if (!m_configDataChange) {
                m_configDataChange = true;
                auto ev = new QEvent(QEvent::UpdateRequest);
                QCoreApplication::postEvent(this, ev);
            }
        }
    });
}

bool Shell::event(QEvent* e)
{
    if (e->type() == QEvent::UpdateRequest) {
        //是否在所有屏幕上显示任务栏，变为true时
        //创建副屏的任务栏
        if(m_showPanelOnAllScreens) {
            for(Screen * screen : m_screenManager->screens()) {
                if(!m_panels.contains(screen)) {
                    auto panel = new Panel(screen, QString("panel%1").arg(m_panels.size()));
                    connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
                    m_panels.insert(screen, panel);
                }
            }
        } else {
            //是否在所有屏幕上显示任务栏，变为false时
            //删除不是主屏任务栏的所有任务栏
            for(Screen * screen : m_screenManager->screens()) {
                if(m_panels.contains(screen) && screen != m_screenManager->primaryScreen()) {
                    delete m_panels.take(screen);
                }
            }
        }
        m_configDataChange = false;
        e->accept();
    }
    return QObject::event(e);
}
