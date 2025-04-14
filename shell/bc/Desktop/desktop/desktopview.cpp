/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "desktopview.h"
#include "dockdbusinterface.h"
#include "thumbnailer/thumbnailprovider.h"

#include <QQmlEngine>
#include <QQmlContext>

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include <KWindowSystem>

DesktopView::DesktopView(QScreen *screen, QQuickView *parent)
    : QQuickView(parent)
    , m_screen(screen)
{
    m_screenRect = m_screen->geometry();
    this->setFlag(Qt::FramelessWindowHint);
    this->setColor(QColor(Qt::transparent));
    KWindowSystem::setType(winId(), NET::Desktop);
    KWindowSystem::setState(winId(), NET::KeepBelow);

    engine()->rootContext()->setContextProperty("desktopView", this);
    engine()->rootContext()->setContextProperty("Dock", DockDBusInterface::self());
    QWindow::fromWinId(winId())->setOpacity(0.99);
    engine()->addImageProvider("thumbnailer", new ThumbnailProvider());

    setTitle(tr("Desktop"));
    setScreen(m_screen);
    setResizeMode(QQuickView::SizeRootObjectToView);

    onGeometryChanged();
    onPrimaryScreenChanged(QGuiApplication::primaryScreen());

    // 主屏改变
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DesktopView::onPrimaryScreenChanged);

    connect(m_screen, &QScreen::virtualGeometryChanged, this, &DesktopView::onGeometryChanged);
    connect(m_screen, &QScreen::geometryChanged, this, &DesktopView::onGeometryChanged);
}

QRect DesktopView::screenRect()
{
    return m_screenRect;
}

void DesktopView::onPrimaryScreenChanged(QScreen *screen)
{
    bool isPrimaryScreen = m_screen->name() == screen->name();

    onGeometryChanged();

    setSource(isPrimaryScreen ? QStringLiteral("qrc:/qml/Main.qml")
                              : QStringLiteral("qrc:/qml/Wallpaper.qml"));
}

void DesktopView::onGeometryChanged()
{
    m_screenRect = m_screen->geometry().adjusted(0, 0, 1, 1);
    setGeometry(m_screenRect);
    emit screenRectChanged();
}
