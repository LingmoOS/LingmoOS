/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QGuiApplication>
#include "screens-manager.h"

namespace LingmoUIPanel {
Screen::Screen(QScreen *screen, QObject *parent): QObject(parent), m_screen(screen)
{
    checkGeometry();
    connect(m_screen, &QScreen::geometryChanged, this, &Screen::onGeometryChanged);
}

QRect Screen::geometry() const
{
    return m_screen->geometry();
}

void Screen::checkGeometry()
{
    for(auto screen : qApp->screens()) {
        if(screen->geometry() == m_screen->geometry() && m_screen != screen && m_screen != qApp->primaryScreen()) {
            if(m_enable) {
                m_enable = false;
                Q_EMIT enableChanged(false);
            }
            return;
        }
    }
    if(!m_enable) {
        m_enable = true;
        Q_EMIT enableChanged(true);
    }
}

void Screen::onGeometryChanged(const QRect &geometry)
{
    checkGeometry();
    if(m_enable) {
        Q_EMIT geometryChanged(geometry);
    }
}

QScreen *Screen::internal()
{
    return m_screen;
}

bool Screen::enable()
{
    return m_enable;
}

ScreensManager::ScreensManager(QObject *parent) : QObject(parent)
{
    for(auto screen : qApp->screens()) {
        auto s = new Screen(screen, this);
        connect(s, &Screen::enableChanged, this, [&, s](bool enable){
            onEnableChanged(s, enable);
        });
        if(screen == qApp->primaryScreen()) {
            m_primaryScreen = s;
        }
        m_screens.append(s);
    }
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreensManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreensManager::onScreenRemoved);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &ScreensManager::onPrimaryScreenChanged);
}

ScreensManager::~ScreensManager()
{
    qDeleteAll(m_screens);
}

QList<Screen *> ScreensManager::screens()
{
    QList<Screen *> tmp;
    for(auto screen : m_screens) {
        if(screen->enable()) {
            tmp.append(screen);
        }
    }
    return tmp;
}

Screen *ScreensManager::primaryScreen()
{
    return m_primaryScreen;
}

void ScreensManager::onScreenAdded(QScreen *screen)
{
    auto s = new Screen(screen, this);
    m_screens.append(s);
    connect(s, &Screen::enableChanged, this, [&, s](bool enable){
        onEnableChanged(s, enable);
    });
    if(s->enable()) {
        Q_EMIT screenAdded(s);
    }
}

void ScreensManager::onScreenRemoved(QScreen *screen)
{
    int i = 0;
    for(; i <= m_screens.size(); ++i) {
        if(m_screens.at(i)->internal() == screen) {
            break;
        }
    }
    if(i < m_screens.size()) {
        auto s = m_screens.takeAt(i);
        Q_EMIT screenRemoved(s);
        s->deleteLater();
    }
}

void ScreensManager::onPrimaryScreenChanged(QScreen *screen)
{
    if(m_primaryScreen->internal() != screen) {
        for(auto s : m_screens) {
            if(s->internal() == screen) {
                m_primaryScreen = s;
                Q_EMIT primaryScreenChanged(s);
                return;
            }
        }
    }
}

void ScreensManager::onEnableChanged(Screen *screen, bool enable)
{
    if(enable) {
        Q_EMIT screenAdded(screen);
    } else {
        Q_EMIT screenRemoved(screen);
    }
}

} // LingmoUIPanel