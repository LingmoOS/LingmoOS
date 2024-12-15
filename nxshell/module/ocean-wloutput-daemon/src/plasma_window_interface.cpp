// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plasma_window_interface.h"
#include <QtDBus/QDBusMessage>
#include <QDBusInterface>

PlasmaWindowInterface::PlasmaWindowInterface(PlasmaWindow* plasma_window)
    : QDBusAbstractAdaptor(plasma_window)
    , m_plasmaWindow(plasma_window)
{
    setAutoRelaySignals(true);

    InitConnect();
}

PlasmaWindowInterface::~PlasmaWindowInterface()
{
}

void PlasmaWindowInterface::InitConnect()
{
    qDebug() << "on plasma Window created, title:" << m_plasmaWindow->title();

    connect(m_plasmaWindow, &PlasmaWindow::activeChanged, this, &PlasmaWindowInterface::ActiveChanged);
    connect(m_plasmaWindow, &PlasmaWindow::appIdChanged, this, &PlasmaWindowInterface::AppIdChanged);
    connect(m_plasmaWindow, &PlasmaWindow::closeableChanged, this, &PlasmaWindowInterface::CloseableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::demandsAttentionChanged, this, &PlasmaWindowInterface::DemandsAttentionChanged);
    connect(m_plasmaWindow, &PlasmaWindow::fullscreenableChanged, this, &PlasmaWindowInterface::FullscreenableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::fullscreenChanged, this, &PlasmaWindowInterface::FullscreenChanged);
    connect(m_plasmaWindow, &PlasmaWindow::geometryChanged, this, &PlasmaWindowInterface::GeometryChanged);
    connect(m_plasmaWindow, &PlasmaWindow::iconChanged, this, &PlasmaWindowInterface::IconChanged);
    connect(m_plasmaWindow, &PlasmaWindow::keepAboveChanged, this, &PlasmaWindowInterface::KeepAboveChanged);
    connect(m_plasmaWindow, &PlasmaWindow::keepBelowChanged, this, &PlasmaWindowInterface::KeepBelowChanged);
    connect(m_plasmaWindow, &PlasmaWindow::maximizeableChanged, this, &PlasmaWindowInterface::MaximizeableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::maximizedChanged, this, &PlasmaWindowInterface::MaximizedChanged);
    connect(m_plasmaWindow, &PlasmaWindow::minimizeableChanged, this, &PlasmaWindowInterface::MinimizeableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::minimizedChanged, this, &PlasmaWindowInterface::MinimizedChanged);
    connect(m_plasmaWindow, &PlasmaWindow::movableChanged, this, &PlasmaWindowInterface::MovableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::onAllDesktopsChanged, this, &PlasmaWindowInterface::OnAllDesktopsChanged);
    connect(m_plasmaWindow, &PlasmaWindow::parentWindowChanged, this, &PlasmaWindowInterface::ParentWindowChanged);
    connect(m_plasmaWindow, &PlasmaWindow::plasmaVirtualDesktopEntered, this, &PlasmaWindowInterface::PlasmaVirtualDesktopEntered);
    connect(m_plasmaWindow, &PlasmaWindow::plasmaVirtualDesktopLeft, this, &PlasmaWindowInterface::PlasmaVirtualDesktopLeft);
    connect(m_plasmaWindow, &PlasmaWindow::resizableChanged, this, &PlasmaWindowInterface::ResizableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::shadeableChanged, this, &PlasmaWindowInterface::ShadeableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::shadedChanged, this, &PlasmaWindowInterface::ShadedChanged);
    connect(m_plasmaWindow, &PlasmaWindow::skipSwitcherChanged, this, &PlasmaWindowInterface::SkipSwitcherChanged);
    connect(m_plasmaWindow, &PlasmaWindow::skipTaskbarChanged, this, &PlasmaWindowInterface::SkipTaskbarChanged);
    connect(m_plasmaWindow, &PlasmaWindow::titleChanged, this, &PlasmaWindowInterface::TitleChanged);
    connect(m_plasmaWindow, &PlasmaWindow::unmapped, this, &PlasmaWindowInterface::Unmapped);
    connect(m_plasmaWindow, &PlasmaWindow::virtualDesktopChangeableChanged, this, &PlasmaWindowInterface::VirtualDesktopChangeableChanged);
    connect(m_plasmaWindow, &PlasmaWindow::virtualDesktopChanged, this, &PlasmaWindowInterface::VirtualDesktopChanged);
}

QString PlasmaWindowInterface::AppId() const
{
    if (m_plasmaWindow != nullptr) {
         return m_plasmaWindow->appId();
    }

    return QString();
}

void PlasmaWindowInterface::Destroy()
{
     if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->destroy();
    }
}

QRect PlasmaWindowInterface::Geometry() const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->geometry();
    }

    return QRect();
}

// 查找最合适的图标大小,参考xutils.go里470行的逻辑
// findBestEwmhIcon takes width/height dimensions and a slice of *ewmh.WmIcon
// and finds the best matching icon of the bunch. We always prefer bigger.
// If no icons are bigger than the preferred dimensions, use the biggest
// available. Otherwise, use the smallest icon that is greater than or equal
// to the preferred dimensions. The preferred dimensions is essentially
// what you'll likely scale the resulting icon to.
// If width and height are 0, then the largest icon found will be returned.
QPixmap findBestEwmhIcon(QIcon icon, int width, int height)
{
    if (icon.isNull())
        return QPixmap();

    int best = -1;
    int parea = width * height;  // preferred size

    // If zero area, set it to the largest possible.
    if (parea == 0) {
        parea = INT_MAX;
    }

    int bestArea = 0;
    int iconArea = 0;
    QList<QSize> sizes = icon.availableSizes();

    for (int i = 0; i < sizes.size(); ++i) {
        // the first valid icon we've seen; use it
        if (best == -1) {
            best = i;
            continue;
        }

        // load areas for comparison
        bestArea = sizes[best].width() * sizes[best].height();
        iconArea = sizes[i].width() * sizes[i].height();

        // We don't always want to accept bigger icons if our best is alread bigger.
        // But we always want something bigger if our best is insufficient.
        if ((iconArea >= parea && iconArea <= bestArea) ||
                (bestArea < parea && iconArea > bestArea))
            best = i;
    }

    if (best > -1)
        return icon.pixmap(sizes[best]);

    return QPixmap();
}

const int bestIconSize = 48;

QString PlasmaWindowInterface::Icon () const
{
    if (m_plasmaWindow != nullptr) {
        QIcon icon = m_plasmaWindow->icon();

        // 查找最合适的图标大小，然后将图片数据转为base64的数据
        QByteArray hexed;
        QPixmap pix = findBestEwmhIcon(icon, bestIconSize, bestIconSize);
        if (!pix.isNull()) {
            QByteArray ba;
            QBuffer buf(&ba);
            pix.save(&buf, "PNG");
            hexed = ba.toBase64();
            buf.close();
        }

        if (hexed.isNull() || hexed.isEmpty())
            return QString();

        // 将base64的图片原始数据组装成Data URL scheme格式
        QString dataUrl;
        dataUrl.append("data:image/png;base64,");
        dataUrl.append(hexed);
        return  dataUrl;
    }

    return QString();
}

quint32 PlasmaWindowInterface::InternalId () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->internalId();
    }

    return 0;
}

bool PlasmaWindowInterface::PlasmaWindowInterface::IsActive () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isActive();
    }

    return false;
}

bool PlasmaWindowInterface::IsCloseable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isCloseable();
    }

    return false;
}

bool PlasmaWindowInterface::IsDemandingAttention () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isDemandingAttention();
    }

    return false;
}

bool PlasmaWindowInterface::IsFullscreen () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isFullscreen();
    }

    return false;
}

bool PlasmaWindowInterface::IsFullscreenable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isFullscreenable();
    }

    return false;
}

bool PlasmaWindowInterface::IsKeepAbove () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isKeepAbove();
    }

    return false;
}

bool PlasmaWindowInterface::IsKeepBelow () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isKeepBelow();
    }

    return false;
}

bool PlasmaWindowInterface::IsMaximizeable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isMaximizeable();
    }

    return false;
}

bool PlasmaWindowInterface::IsMaximized () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isMaximized();
    }

    return false;
}

bool PlasmaWindowInterface::IsMinimizeable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isMinimizeable();
    }

    return false;
}

bool PlasmaWindowInterface::IsMinimized () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isMinimized();
    }

    return false;
}

bool PlasmaWindowInterface::IsMovable () const
{
    if(m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isMovable();
    }

    return false;
}

bool PlasmaWindowInterface::IsOnAllDesktops () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isOnAllDesktops();
    }

    return false;
}

bool PlasmaWindowInterface::IsResizable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isResizable();
    }

    return false;
}

bool PlasmaWindowInterface::IsShadeable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isShadeable();
    }

    return false;
}

bool PlasmaWindowInterface::IsShaded () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isShaded();
    }

    return false;
}

bool PlasmaWindowInterface::IsValid () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isValid();
    }

    return false;
}

bool PlasmaWindowInterface::IsVirtualDesktopChangeable () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->isVirtualDesktopChangeable();
    }

    return false;
}

quint32 PlasmaWindowInterface::Pid () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->pid();
    }

    return 0;
}

QStringList PlasmaWindowInterface::PlasmaVirtualDesktops () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->plasmaVirtualDesktops();
    }

    return QStringList();
}

void PlasmaWindowInterface::Release ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->release();
    }
}

void PlasmaWindowInterface::RequestActivate ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestActivate();
    }
}

void PlasmaWindowInterface::RequestClose ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestClose();
    }
}

void PlasmaWindowInterface::RequestEnterNewVirtualDesktop ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestEnterNewVirtualDesktop();
    }
}

void PlasmaWindowInterface::RequestEnterVirtualDesktop (const QString &id)
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestEnterVirtualDesktop(id);
    }
}

void PlasmaWindowInterface::RequestLeaveVirtualDesktop (const QString &id)
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestLeaveVirtualDesktop(id);
    }
}

void PlasmaWindowInterface::RequestMove ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestMove();
    }
}

void PlasmaWindowInterface::RequestResize ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestResize();
    }
}

void PlasmaWindowInterface::RequestToggleKeepAbove ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestToggleKeepAbove();
    }
}

void PlasmaWindowInterface::RequestToggleKeepBelow ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestToggleKeepBelow();
    }
}

void PlasmaWindowInterface::RequestToggleMaximized ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestToggleMaximized();
    }
}

void PlasmaWindowInterface::RequestToggleMinimized ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestToggleMinimized();
    }
}

void PlasmaWindowInterface::RequestToggleShaded ()
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestToggleShaded();
    }
}

void PlasmaWindowInterface::RequestVirtualDesktop (quint32 desktop)
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->requestVirtualDesktop(desktop);
    }
}

void PlasmaWindowInterface::SetMinimizedGeometry (Surface *panel, const QRect &geom)
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->setMinimizedGeometry(panel, geom);
    }
}

bool PlasmaWindowInterface::SkipSwitcher () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->skipSwitcher();
    }

    return false;
}

bool PlasmaWindowInterface::SkipTaskbar () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->skipTaskbar();
    }

    return false;
}

QString PlasmaWindowInterface::Title () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->title();
    }

    return QString();
}

void PlasmaWindowInterface::UnsetMinimizedGeometry (Surface *panel)
{
    if (m_plasmaWindow != nullptr) {
        m_plasmaWindow->unsetMinimizedGeometry(panel);
    }
}

quint32 PlasmaWindowInterface::VirtualDesktop () const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->virtualDesktop();
    }

    return 0;
}

quint32 PlasmaWindowInterface::WindowId() const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->windowId();
    }
    return 0;
}

QByteArray PlasmaWindowInterface::uuid() const
{
    if (m_plasmaWindow != nullptr) {
        return m_plasmaWindow->uuid();
    }
    return QByteArray();
}
