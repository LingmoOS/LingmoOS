/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include "launcher.h"
#include "launcheradaptor.h"
#include "iconthemeimageprovider.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QPixmapCache>
#include <QQmlContext>
#include <QScreen>
#include <QTimer>

#include <KWindowSystem>

Launcher::Launcher(bool firstShow, QQuickView *w)
    : QQuickView(w)
    , m_dockInterface("com.lingmo.Dock",
                    "/Dock",
                    "com.lingmo.Dock", QDBusConnection::sessionBus())
    , m_hideTimer(new QTimer)
    , m_showed(false)
    , m_leftMargin(0)
    , m_rightMargin(0)
    , m_bottomMargin(0)
{
    new LauncherAdaptor(this);

    engine()->rootContext()->setContextProperty("launcher", this);

    setColor(Qt::transparent);
    setFlags(Qt::FramelessWindowHint);
    setResizeMode(QQuickView::SizeRootObjectToView);
    setClearBeforeRendering(true);
    onGeometryChanged();

    setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    setTitle(tr("Launcher"));
    setVisible(true);
    setVisible(firstShow);

    // Let the animation in qml be hidden after the execution is complete
    m_hideTimer->setInterval(200);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, &QTimer::timeout, this, [=] { setVisible(false); });

    if (m_dockInterface.isValid() && !m_dockInterface.lastError().isValid()) {
        updateMargins();
        connect(&m_dockInterface, SIGNAL(primaryGeometryChanged()), this, SLOT(updateMargins()));
        connect(&m_dockInterface, SIGNAL(directionChanged()), this, SLOT(updateMargins()));
    } else {
        QDBusServiceWatcher *watcher = new QDBusServiceWatcher("com.lingmo.Dock",
                                                               QDBusConnection::sessionBus(),
                                                               QDBusServiceWatcher::WatchForUnregistration,
                                                               this);
        connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, [=] {
            updateMargins();
            connect(&m_dockInterface, SIGNAL(primaryGeometryChanged()), this, SLOT(updateMargins()));
            connect(&m_dockInterface, SIGNAL(directionChanged()), this, SLOT(updateMargins()));
        });
    }

    connect(qApp, &QApplication::primaryScreenChanged, this, [=] { onGeometryChanged(); });
    connect(this, &QQuickView::activeChanged, this, &Launcher::onActiveChanged);
}

int Launcher::leftMargin() const
{
    return m_leftMargin;
}

int Launcher::rightMargin() const
{
    return m_rightMargin;
}

int Launcher::bottomMargin() const
{
    return m_bottomMargin;
}

bool Launcher::showed()
{
    return m_showed;
}

void Launcher::showWindow()
{
    updateSize();
    m_showed = true;
    emit showedChanged();

    setVisible(true);
}

void Launcher::hideWindow()
{
    setVisible(false);
    m_showed = false;
    emit showedChanged();
}

void Launcher::toggle()
{
    isVisible() ? Launcher::hideWindow() : Launcher::showWindow();
}

bool Launcher::dockAvailable()
{
    return m_dockInterface.isValid();
}

bool Launcher::isPinedDock(const QString &desktop)
{
    QDBusInterface iface("com.lingmo.Dock",
                         "/Dock",
                         "com.lingmo.Dock",
                         QDBusConnection::sessionBus());

    if (!iface.isValid())
        return false;

    return iface.call("pinned", desktop).arguments().first().toBool();
}

void Launcher::clearPixmapCache()
{
    QPixmapCache::clear();
}

QRect Launcher::screenRect()
{
    return m_screenRect;
}

void Launcher::updateMargins()
{
    QRect dockGeometry = m_dockInterface.property("primaryGeometry").toRect();
    int dockDirection = m_dockInterface.property("direction").toInt();

    m_leftMargin = 0;
    m_rightMargin = 0;
    m_bottomMargin = 0;

    if (dockDirection == 0) {
        m_leftMargin = dockGeometry.width();
    } else if (dockDirection == 1) {
        m_bottomMargin = dockGeometry.height();
    } else if (dockDirection == 2) {
        m_rightMargin = dockGeometry.width();
    }

    emit marginsChanged();
}

void Launcher::updateSize()
{
    if (m_screenRect != qApp->primaryScreen()->geometry()) {
        m_screenRect = qApp->primaryScreen()->geometry();
        setGeometry(m_screenRect);
        emit screenRectChanged();
    }
}

void Launcher::onGeometryChanged()
{
    disconnect(screen());

    setScreen(qApp->primaryScreen());
    updateSize();

    connect(screen(), &QScreen::virtualGeometryChanged, this, &Launcher::updateSize);
    connect(screen(), &QScreen::geometryChanged, this, &Launcher::updateSize);
}

void Launcher::showEvent(QShowEvent *e)
{
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);

    QQuickView::showEvent(e);
}

void Launcher::resizeEvent(QResizeEvent *e)
{
    // The window manager forces the size.
    e->ignore();
}

void Launcher::onActiveChanged()
{
    if (!isActive())
        Launcher::hide();
}

