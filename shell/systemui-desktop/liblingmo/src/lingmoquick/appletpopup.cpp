/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletpopup.h"

#include <QGuiApplication>
#include <QQmlProperty>
#include <qpa/qplatformwindow.h> // for QWINDOWSIZE_MAX

#include <KConfigGroup>
#include <KWindowSystem>
#include <KX11Extras>
#include <QSize>

#include "applet.h"
#include "appletquickitem.h"
#include "edgeeventforwarder.h"
#include "lingmoshellwaylandintegration.h"
#include "windowresizehandler.h"

// used in detecting if focus passes to config UI
#include "configview.h"
#include "sharedqmlengine.h"
#include "utils.h"

// This is a proxy object that connects to the Layout attached property of an item
// it also handles turning properties to proper defaults
// we need a wrapper as QQmlProperty can't disconnect

namespace LingmoQuick
{

class LayoutChangedProxy : public QObject
{
    Q_OBJECT
public:
    LayoutChangedProxy(QQuickItem *item);
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize implicitSize() const;
Q_SIGNALS:
    void implicitSizeChanged();
    void minimumSizeChanged();
    void maximumSizeChanged();

private:
    QQmlProperty m_minimumWidth;
    QQmlProperty m_maximumWidth;
    QQmlProperty m_minimumHeight;
    QQmlProperty m_maximumHeight;
    QQmlProperty m_preferredWidth;
    QQmlProperty m_preferredHeight;
    QPointer<QQuickItem> m_item;
};
}

using namespace LingmoQuick;

AppletPopup::AppletPopup()
    : PopupLingmoWindow()
{
    setAnimated(true);
    setFlags(flags() | Qt::Dialog);

    if (KWindowSystem::isPlatformX11()) {
        KX11Extras::setType(winId(), NET::AppletPopup);
    } else {
        LingmoShellWaylandIntegration::get(this)->setRole(QtWayland::org_kde_lingmo_surface::role::role_appletpopup);
    }

    auto edgeForwarder = new EdgeEventForwarder(this);
    edgeForwarder->setMargins(padding());
    connect(this, &LingmoWindow::paddingChanged, this, [edgeForwarder, this]() {
        edgeForwarder->setMargins(padding());
    });
    // edges that have a border are not on a screen edge
    // we want to forward on sides touching screen edges
    edgeForwarder->setActiveEdges(~borders());
    connect(this, &LingmoWindow::bordersChanged, this, [edgeForwarder, this]() {
        edgeForwarder->setActiveEdges(~borders());
    });

    auto windowResizer = new WindowResizeHandler(this);
    windowResizer->setMargins(padding());
    connect(this, &LingmoWindow::paddingChanged, this, [windowResizer, this]() {
        windowResizer->setMargins(padding());
    });

    auto updateWindowResizerEdges = [windowResizer, this]() {
        Qt::Edges activeEdges = borders();
        activeEdges.setFlag(LingmoQuickPrivate::oppositeEdge(effectivePopupDirection()), false);
        windowResizer->setActiveEdges(activeEdges);
    };
    updateWindowResizerEdges();
    connect(this, &LingmoWindow::bordersChanged, this, updateWindowResizerEdges);
    connect(this, &PopupLingmoWindow::effectivePopupDirectionChanged, this, updateWindowResizerEdges);

    connect(this, &LingmoWindow::mainItemChanged, this, &AppletPopup::onMainItemChanged);
    connect(this, &LingmoWindow::paddingChanged, this, &AppletPopup::updateMaxSize);
    connect(this, &LingmoWindow::paddingChanged, this, &AppletPopup::updateSize);
    connect(this, &LingmoWindow::paddingChanged, this, &AppletPopup::updateMinSize);

    connect(this, &LingmoWindow::screenChanged, this, [this](QScreen *screen) {
        if (m_oldScreen) {
            disconnect(m_oldScreen, &QScreen::geometryChanged, this, &AppletPopup::updateMaxSize);
        }
        if (screen) {
            connect(screen, &QScreen::geometryChanged, this, &AppletPopup::updateMaxSize);
        }
        m_oldScreen = screen;
        updateMaxSize();
    });
}

AppletPopup::~AppletPopup()
{
}

QQuickItem *AppletPopup::appletInterface() const
{
    return m_appletInterface.data();
}

void AppletPopup::setAppletInterface(QQuickItem *appletInterface)
{
    if (appletInterface == m_appletInterface) {
        return;
    }

    m_appletInterface = qobject_cast<AppletQuickItem *>(appletInterface);
    m_sizeExplicitlySetFromConfig = false;

    if (m_appletInterface) {
        KConfigGroup config = m_appletInterface->applet()->config();
        QSize size;
        size.rwidth() = config.readEntry("popupWidth", 0);
        size.rheight() = config.readEntry("popupHeight", 0);
        if (!size.isEmpty()) {
            m_sizeExplicitlySetFromConfig = true;
            resize(size.grownBy(padding()));
            return;
        }
    }

    Q_EMIT appletInterfaceChanged();
}

bool AppletPopup::hideOnWindowDeactivate() const
{
    return m_hideOnWindowDeactivate;
}

void AppletPopup::setHideOnWindowDeactivate(bool hideOnWindowDeactivate)
{
    if (hideOnWindowDeactivate == m_hideOnWindowDeactivate) {
        return;
    }
    m_hideOnWindowDeactivate = hideOnWindowDeactivate;
    Q_EMIT hideOnWindowDeactivateChanged();
}

void AppletPopup::hideEvent(QHideEvent *event)
{
    // Persist the size if this contains an applet
    if (m_appletInterface) {
        KConfigGroup config = m_appletInterface->applet()->config();
        // save size without margins, so we're robust against theme changes
        const QSize popupSize = size().shrunkBy(padding());
        config.writeEntry("popupWidth", popupSize.width());
        config.writeEntry("popupHeight", popupSize.height());
        config.sync();
    }

    PopupLingmoWindow::hideEvent(event);
}

void AppletPopup::focusOutEvent(QFocusEvent *ev)
{
    if (m_hideOnWindowDeactivate) {
        bool parentHasFocus = false;

        QWindow *parentWindow = transientParent();

        while (parentWindow) {
            if (parentWindow->isActive() && !(parentWindow->flags() & Qt::WindowDoesNotAcceptFocus)) {
                parentHasFocus = true;
                break;
            }

            parentWindow = parentWindow->transientParent();
        }

        const QWindow *focusWindow = QGuiApplication::focusWindow();
        bool childHasFocus = focusWindow && ((focusWindow->isActive() && isAncestorOf(focusWindow)) || (focusWindow->type() & Qt::Popup) == Qt::Popup);

        const bool viewClicked = qobject_cast<const LingmoQuick::SharedQmlEngine *>(focusWindow) || qobject_cast<const ConfigView *>(focusWindow);

        if (viewClicked || (!parentHasFocus && !childHasFocus)) {
            setVisible(false);
        }
    }

    PopupLingmoWindow::focusOutEvent(ev);
}

void AppletPopup::onMainItemChanged()
{
    QQuickItem *mainItem = LingmoWindow::mainItem();
    if (!mainItem) {
        m_layoutChangedProxy.reset();
        return;
    }

    // update window to mainItem size hints
    m_layoutChangedProxy.reset(new LayoutChangedProxy(mainItem));
    connect(m_layoutChangedProxy.data(), &LayoutChangedProxy::maximumSizeChanged, this, &AppletPopup::updateMaxSize);
    connect(m_layoutChangedProxy.data(), &LayoutChangedProxy::minimumSizeChanged, this, &AppletPopup::updateMinSize);
    connect(m_layoutChangedProxy.data(), &LayoutChangedProxy::implicitSizeChanged, this, &AppletPopup::updateSize);

    updateMinSize();
    updateMaxSize();
    updateSize();
}

void AppletPopup::updateMinSize()
{
    if (!m_layoutChangedProxy) {
        return;
    }
    setMinimumSize(m_layoutChangedProxy->minimumSize().grownBy(padding()));
    // SetMinimumsize doesn't work since
    // https://codereview.qt-project.org/c/qt/qtwayland/+/527831
    // which fixes and conforms to the wayland protocol specification.
    // This workaround is needed as the bug is in the protocol itself
    resize(std::max(size().width(), minimumSize().width()), std::max(size().height(), minimumSize().height()));
}

void AppletPopup::updateMaxSize()
{
    if (!m_layoutChangedProxy) {
        return;
    }
    QSize maxSize = m_layoutChangedProxy->maximumSize().grownBy(padding());
    if (screen()) {
        maxSize.setWidth(std::min(maxSize.width(), int(std::round(screen()->geometry().width() * 0.95))));
        maxSize.setHeight(std::min(maxSize.height(), int(std::round(screen()->geometry().height() * 0.95))));
    }
    setMaximumSize(maxSize);
    resize(std::min(size().width(), maxSize.width()), std::min(size().height(), maxSize.height()));
}

void AppletPopup::updateSize()
{
    if (m_sizeExplicitlySetFromConfig) {
        return;
    }
    if (!m_layoutChangedProxy) {
        return;
    }
    const QSize wantedSize = m_layoutChangedProxy->implicitSize().grownBy(padding());

    // NOTE: not using std::clamp as it might assert due to (possible) malformed values, sich as min > max
    QSize size = {
        std::min(std::max(minimumSize().width(), wantedSize.width()), maximumSize().width()),
        std::min(std::max(minimumSize().height(), wantedSize.height()), maximumSize().height())
    };

    resize(size);
}

LayoutChangedProxy::LayoutChangedProxy(QQuickItem *item)
    : m_item(item)
{
    m_minimumWidth = QQmlProperty(item, QStringLiteral("Layout.minimumWidth"), qmlContext(item));
    m_minimumHeight = QQmlProperty(item, QStringLiteral("Layout.minimumHeight"), qmlContext(item));
    m_maximumWidth = QQmlProperty(item, QStringLiteral("Layout.maximumWidth"), qmlContext(item));
    m_maximumHeight = QQmlProperty(item, QStringLiteral("Layout.maximumHeight"), qmlContext(item));
    m_preferredWidth = QQmlProperty(item, QStringLiteral("Layout.preferredWidth"), qmlContext(item));
    m_preferredHeight = QQmlProperty(item, QStringLiteral("Layout.preferredHeight"), qmlContext(item));

    m_minimumWidth.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::minimumSizeChanged).methodIndex());
    m_minimumHeight.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::minimumSizeChanged).methodIndex());
    m_maximumWidth.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::maximumSizeChanged).methodIndex());
    m_maximumHeight.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::maximumSizeChanged).methodIndex());
    m_preferredWidth.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::implicitSizeChanged).methodIndex());
    m_preferredHeight.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::implicitSizeChanged).methodIndex());
    connect(item, &QQuickItem::implicitWidthChanged, this, &LayoutChangedProxy::implicitSizeChanged);
    connect(item, &QQuickItem::implicitHeightChanged, this, &LayoutChangedProxy::implicitSizeChanged);
}

QSize LayoutChangedProxy::maximumSize() const
{
    QSize size(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX);
    qreal width = m_maximumWidth.read().toReal();
    if (qIsFinite(width) && int(width) > 0) {
        size.setWidth(width);
    }
    qreal height = m_maximumHeight.read().toReal();
    if (qIsFinite(height) && int(height) > 0) {
        size.setHeight(height);
    }

    return size;
}

QSize LayoutChangedProxy::implicitSize() const
{
    QSize size(200, 200);

    // Layout.preferredSize has precedent over implicit in layouts
    // so mimic that behaviour here
    if (m_item) {
        size = QSize(m_item->implicitWidth(), m_item->implicitHeight());
    }
    qreal width = m_preferredWidth.read().toReal();
    if (qIsFinite(width) && int(width) > 0) {
        size.setWidth(width);
    }
    qreal height = m_preferredHeight.read().toReal();
    if (qIsFinite(height) && int(height) > 0) {
        size.setHeight(height);
    }
    return size;
}

QSize LayoutChangedProxy::minimumSize() const
{
    QSize size(0, 0);
    qreal width = m_minimumWidth.read().toReal();
    if (qIsFinite(width) && int(width) > 0) {
        size.setWidth(width);
    }
    qreal height = m_minimumHeight.read().toReal();
    if (qIsFinite(height) && int(height) > 0) {
        size.setHeight(height);
    }

    return size;
}

#include "appletpopup.moc"
