/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dialog.h"
#include <QScreen>
#include <QGuiApplication>
#include <KWindowInfo>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>
#include <KWindowEffects>
#include <QTimer>

#include <windowmanager/windowmanager.h>
#include <windowmanager/lingmoshell.h>
#include <lingmostylehelper/lingmo-decoration-manager.h>
#include <lingmostylehelper/xatom-helper.h>

#include "wayland-integration_p.h"

namespace LingmoUIQuick {

#define QWINDOWSIZE_MAX ((1<<24)-1)
class DialogPrivate
{
public:
    explicit DialogPrivate(Dialog *dialog);

    /**
     * This function is an optimized version of updateMaximumHeight,
     * updateMaximumWidth,updateMinimumWidth and updateMinimumHeight.
     * It should be called when you need to call all 4 of these functions
     * AND you have called syncToMainItemSize before.
     */
    void updateLayoutParameters();
    void syncToMainItemSize();
    void slotMainItemSizeChanged();
    void slotWindowPositionChanged();
    void updateMinimumWidth();
    void updateMinimumHeight();
    void updateMaximumWidth();
    void updateMaximumHeight();
    /**
     * Gets the maximum and minimum size hints for the window based on the contents. it doesn't actually resize anything
     */
    void getSizeHints(QSize &min, QSize &max) const;
    /**
     * This function checks the current position of the dialog and repositions
     * it so that no part of it is not on the screen
     */
    void repositionIfOffScreen();
    QRect availableScreenGeometryForPosition(const QPoint &pos) const;
    void updateTheme();
    void updateVisibility(bool visible);
    void applyType();
    QPointF positionAdjustedForMainItem(const QPointF &point) const;
    bool mainItemContainsPosition(const QPointF &point) const;
    void updateBlurRegion();

    Dialog *q;
    QPointer<QQuickItem> m_mainItem;
    QPointer<QObject> m_mainItemLayout;
    QPointer<QQuickItem> m_visualParent;
    bool m_componentComplete;
    LingmoUIQuick::Dialog::PopupLocation m_location;
    bool m_hideOnWindowDeactivate;
    bool m_visible;
    QPointer<WindowProxy> m_windowProxy;
    QRect m_cachedGeometry;
    WindowType::Type m_type;
    QRegion m_windowBlurRegion;
    bool m_enableWindowBlur;
    int m_margin;
    bool m_removeHeaderBar;
};

DialogPrivate::DialogPrivate(Dialog *dialog)
        : q(dialog)
        , m_location(LingmoUIQuick::Dialog::PopupLocation::BottomEdge)
        , m_hideOnWindowDeactivate(false)
        , m_visible(false)
        , m_componentComplete(dialog->parent() == nullptr)
        , m_type(WindowType::Type::Normal)
        , m_enableWindowBlur(false)
        , m_margin(0)
        , m_removeHeaderBar{false}
{
}

void DialogPrivate::slotMainItemSizeChanged()
{
    syncToMainItemSize();
}

void DialogPrivate::slotWindowPositionChanged()
{
    // Tooltips always have all the borders
    // floating windows have all borders
    if (!q->isVisible() || q->flags().testFlag(Qt::ToolTip) || m_location == LingmoUIQuick::Dialog::PopupLocation::Floating) {
        return;
    }
    updateTheme();

    if (m_mainItem) {
        m_mainItem->setPosition(QPoint(0, 0));
        m_mainItem->setSize(QSize(q->width(), q->height()));
    }
}

void DialogPrivate::updateMinimumWidth()
{
    if(!m_mainItem || !m_mainItemLayout) {
        return;
    }

    if (!m_componentComplete) {
        return;
    }

    q->setMinimumWidth(0);
    int minimumWidth = m_mainItemLayout->property("minimumWidth").toInt();
    if (q->screen()) {
        minimumWidth = qMin(q->screen()->availableGeometry().width(), minimumWidth);
    }
    q->contentItem()->setWidth(qMax(q->width(), minimumWidth));
    q->setWidth(qMax(q->width(), minimumWidth));

    updateLayoutParameters();
}

void DialogPrivate::updateMinimumHeight()
{
    if(!m_mainItem || !m_mainItemLayout) {
        return;
    }

    if (!m_componentComplete) {
        return;
    }

    q->setMinimumHeight(0);

    int minimumHeight = m_mainItemLayout->property("minimumHeight").toInt();
    if (q->screen()) {
        minimumHeight = qMin(q->screen()->availableGeometry().height(), minimumHeight);
    }
    q->contentItem()->setHeight(qMax(q->height(), minimumHeight));
    q->setHeight(qMax(q->height(), minimumHeight));

    updateLayoutParameters();
}

void DialogPrivate::updateMaximumWidth()
{
    if(!m_mainItem || !m_mainItemLayout) {
        return;
    }

    if (!m_componentComplete) {
        return;
    }

    q->setMaximumWidth(QWINDOWSIZE_MAX);

    int maximumWidth = m_mainItemLayout->property("maximumWidth").toInt();
    if (q->screen()) {
        maximumWidth = qMin(q->screen()->availableGeometry().width(), maximumWidth);
    }
    q->contentItem()->setWidth(qMin(q->width(), maximumWidth));
    q->setWidth(qMin(q->width(), maximumWidth));

    updateLayoutParameters();
}

void DialogPrivate::updateMaximumHeight()
{
    if(!m_mainItem || !m_mainItemLayout) {
        return;
    }

    if (!m_componentComplete) {
        return;
    }

    q->setMaximumHeight(QWINDOWSIZE_MAX);

    int maximumHeight = m_mainItemLayout->property("maximumHeight").toInt();
    if (q->screen()) {
        maximumHeight = qMin(q->screen()->availableGeometry().height(), maximumHeight);
    }
    q->contentItem()->setHeight(qMin(q->height(), maximumHeight));
    q->setHeight(qMin(q->height(), maximumHeight));

    updateLayoutParameters();
}

void DialogPrivate::updateLayoutParameters()
{
    if (!m_componentComplete || !m_mainItem || !m_mainItemLayout) {
        return;
    }

    m_mainItem->disconnect(q);

    QSize min;
    QSize max(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX);
    getSizeHints(min, max);
    q->setMinimumSize(min);
    q->setMaximumSize(max);
    const QSize finalSize(qBound(min.width(), q->width(), max.width()), qBound(min.height(), q->height(), max.height()));

    if (m_visualParent) {
        // it's important here that we're using re->size() as size, we don't want to do recursive resizeEvents
        const QRect geom(q->popupPosition(m_visualParent, finalSize), finalSize);
        q->adjustGeometry(geom);
    } else {
        q->resize(finalSize);
    }

    m_mainItem->setPosition(QPointF(0, 0));
    m_mainItem->setSize(QSizeF(q->width(), q->height()));

    repositionIfOffScreen();
    updateTheme();

    QObject::connect(m_mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(m_mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::syncToMainItemSize()
{
    if(!m_mainItem) {
        return;
    }

    if (!m_componentComplete) {
        return;
    }
    if (m_mainItem->width() <= 0 || m_mainItem->height() <= 0) {
        qWarning() << "trying to show an empty dialog";
    }


    QSize s = QSize(m_mainItem->width(), m_mainItem->height());

    QSize min;
    QSize max(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX);
    getSizeHints(min, max);
    q->setMinimumSize(min);
    q->setMaximumSize(max);
    s = QSize(qBound(min.width(), s.width(), max.width()), qBound(min.height(), s.height(), max.height()));

    q->contentItem()->setSize(s);

    if (m_visualParent) {
        const QRect geom(q->popupPosition(m_visualParent, s), s);

        if (geom == q->geometry()) {
            return;
        }
        q->adjustGeometry(geom);
    } else {
        q->resize(s);
    }

    m_mainItem->setPosition(QPointF(0, 0));

    updateTheme();
}
void DialogPrivate::getSizeHints(QSize &min, QSize &max) const
{
    if (!m_componentComplete || !m_mainItem || !m_mainItemLayout) {
        return;
    }

    int minimumHeight = m_mainItemLayout->property("minimumHeight").toInt();
    int maximumHeight = m_mainItemLayout->property("maximumHeight").toInt();
    maximumHeight = maximumHeight ? maximumHeight : QWINDOWSIZE_MAX;

    int minimumWidth = m_mainItemLayout->property("minimumWidth").toInt();
    int maximumWidth = m_mainItemLayout->property("maximumWidth").toInt();
    maximumWidth = maximumWidth ? maximumWidth : QWINDOWSIZE_MAX;

    if (q->screen()) {
        minimumWidth = qMin(q->screen()->availableGeometry().width(), minimumWidth);
        minimumHeight = qMin(q->screen()->availableGeometry().height(), minimumHeight);
        maximumWidth = qMin(q->screen()->availableGeometry().width(), maximumWidth);
        maximumHeight = qMin(q->screen()->availableGeometry().height(), maximumHeight);
    }

    min = QSize(minimumWidth, minimumHeight);
    max = QSize(maximumWidth, maximumHeight);
}

void DialogPrivate::repositionIfOffScreen()
{
    if (!m_componentComplete) {
        return;
    }
    const QRect avail = availableScreenGeometryForPosition(q->position());

    int x = q->x();
    int y = q->y();

    if (x < avail.left()) {
        x = avail.left();
    } else if (x + q->width() > avail.right()) {
        x = avail.right() - q->width() + 1;
    }

    if (y < avail.top()) {
        y = avail.top();
    } else if (y + q->height() > avail.bottom()) {
        y = avail.bottom() - q->height() + 1;
    }
    if(m_windowProxy) {
        m_windowProxy->setGeometry(QRect(x, y, q->width(), q->height()));
    }
}

QRect DialogPrivate::availableScreenGeometryForPosition(const QPoint &pos) const
{
    // FIXME: QWindow::screen() never ever changes if the window is moved across
    //        virtual screens (normal two screens with X), this seems to be intentional
    //        as it's explicitly mentioned in the docs. Until that's changed or some
    //        more proper way of howto get the current QScreen for given QWindow is found,
    //        we simply iterate over the virtual screens and pick the one our QWindow
    //        says it's at.
    QRect avail;
    const auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        // we check geometry() but then take availableGeometry()
        // to reliably check in which screen a position is, we need the full
        // geometry, including areas for panels
        if (screen->geometry().contains(pos)) {
            avail = screen->availableGeometry();
            break;
        }
    }

    /*
     * if the heuristic fails (because the topleft of the dialog is offscreen)
     * use at least our screen()
     * the screen should be correctly updated now on Qt 5.3+ so should be
     * more reliable anyways (could be tried to remove the whole for loop
     * above at this point)
     *
     * important: screen can be a nullptr... see bug 345173
     */
    if (avail.isEmpty() && q->screen()) {
        avail = q->screen()->availableGeometry();
    }

    return avail;
}

void DialogPrivate::updateTheme()
{
}

void DialogPrivate::updateVisibility(bool visible)
{
    if (visible) {
        if (m_visualParent && m_visualParent->window()) {
            q->setTransientParent(m_visualParent->window());
        }

        if (q->location() == LingmoUIQuick::Dialog::PopupLocation::FullScreen) {
            // We cache the original size of the item, to retrieve it
            // when the dialog is switched back from fullscreen.
            if (q->geometry() != q->screen()->availableGeometry()) {
                m_cachedGeometry = q->geometry();
            }
            q->setGeometry(q->screen()->availableGeometry());
        } else {
            if (!m_cachedGeometry.isNull()) {
                q->resize(m_cachedGeometry.size());
                slotWindowPositionChanged();
                if (m_visualParent) {
                    q->setPosition(q->popupPosition(m_visualParent, q->size()));
                }
                m_cachedGeometry = QRect();
            }

            if (m_mainItem) {
                syncToMainItemSize();
            }
            if (m_mainItemLayout) {
                updateLayoutParameters();
            }

            // if is a wayland window that was hidden, we need
            // to set its position again as there won't be any move event to sync QWindow::position and shellsurface::position
            if (q->type() != LingmoUIQuick::WindowType::OnScreenDisplay) {
                if(m_windowProxy) {
                    m_windowProxy->setPosition(q->position());
                }
            }
        }
        q->raise();
        applyType();
    }
}

void DialogPrivate::applyType()
{
    if(m_windowProxy) {
        m_windowProxy->setWindowType(m_type);
    }
}

QPointF DialogPrivate::positionAdjustedForMainItem(const QPointF &point) const
{
    if (!m_mainItem) {
        return point;
    }

    QRectF itemRect(m_mainItem->mapToScene(QPoint(0, 0)), QSizeF(m_mainItem->width(), m_mainItem->height()));

    return QPointF(qBound(itemRect.left(), point.x(), itemRect.right()), qBound(itemRect.top(), point.y(), itemRect.bottom()));
}

bool DialogPrivate::mainItemContainsPosition(const QPointF &point) const
{
    if (!m_mainItem) {
        return false;
    }

    return QRectF(m_mainItem->mapToScene(QPoint(0, 0)), QSizeF(m_mainItem->width(), m_mainItem->height())).contains(point);
}

void DialogPrivate::updateBlurRegion()
{
    KWindowEffects::enableBlurBehind(q, m_enableWindowBlur, m_windowBlurRegion);
}

Dialog::Dialog(QQuickItem *parent) : QQuickWindow(parent ? parent->window() : nullptr), d(new DialogPrivate(this))
{
    setColor(QColor(Qt::transparent));
    setFlags(Qt::Dialog);
    connect(this, &QWindow::xChanged, [=]() {
        d->slotWindowPositionChanged();
    });
    connect(this, &QWindow::yChanged, [=]() {
        d->slotWindowPositionChanged();
    });
    // Given dialogs are skip task bar and don't have a decoration
    // minimizing them using e.g. "minimize all" should just close them
    connect(this, &QWindow::windowStateChanged, this, [this](Qt::WindowState newState) {
        if (newState == Qt::WindowMinimized) {
            setVisible(false);
        }
    });

    connect(this, &QWindow::visibleChanged, this, &Dialog::visibleChangedProxy);

    // HACK: this property is invoked due to the initialization that gets done to contentItem() in the getter
    property("data");
    d->m_windowProxy = new WindowProxy(this);
}

Dialog::~Dialog()
{
    disconnect(this, nullptr, this, nullptr);
}

QQuickItem *Dialog::mainItem() const
{
    return d->m_mainItem;
}

void Dialog::setMainItem(QQuickItem *mainItem)
{
    if (d->m_mainItem != mainItem) {
        if (d->m_mainItem) {
            disconnect(d->m_mainItem, nullptr, this, nullptr);
            d->m_mainItem->setParentItem(nullptr);
        }

        if (d->m_mainItemLayout) {
            disconnect(d->m_mainItemLayout, nullptr, this, nullptr);
        }

        d->m_mainItem = mainItem;

        if (mainItem) {
            mainItem->setParentItem(contentItem());

            connect(mainItem, &QQuickItem::widthChanged, this, [&](){
                d->slotMainItemSizeChanged();
            });
            connect(mainItem, &QQuickItem::heightChanged, this, [&](){
                d->slotMainItemSizeChanged();
            });
            d->slotMainItemSizeChanged();

            // Extract the representation's Layout, if any
            QObject *layout = nullptr;
            setMinimumSize(QSize(0, 0));
            setMaximumSize(QSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX));

            // Search a child that has the needed Layout properties
            // HACK: here we are not type safe, but is the only way to access to a pointer of Layout
            const auto lstChild = mainItem->children();
            for (QObject *child : lstChild) {
                // find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
                if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() && child->property("preferredWidth").isValid()
                    && child->property("preferredHeight").isValid() && child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid()
                    && child->property("fillWidth").isValid() && child->property("fillHeight").isValid()) {
                    layout = child;
                    break;
                }
            }

            d->m_mainItemLayout = layout;

            if (layout) {
                // These connections are direct. They run on the GUI thread.
                // If the underlying QQuickItem is sane, these properties should be updated atomically in one cycle
                // of the GUI thread event loop, denying the chance for the event loop to run a QQuickItem::update() call in between.
                // So we avoid rendering a frame in between with inconsistent geometry properties which would cause flickering issues.
                connect(layout, SIGNAL(minimumWidthChanged()), this, SLOT(updateMinimumWidth()));
                connect(layout, SIGNAL(minimumHeightChanged()), this, SLOT(updateMinimumHeight()));
                connect(layout, SIGNAL(maximumWidthChanged()), this, SLOT(updateMaximumWidth()));
                connect(layout, SIGNAL(maximumHeightChanged()), this, SLOT(updateMaximumHeight()));

                d->updateLayoutParameters();
            }
        }

        // if this is called in Component.onCompleted we have to wait a loop the item is added to a scene
        Q_EMIT mainItemChanged();
    }
}

QQuickItem *Dialog::visualParent() const
{
    return d->m_visualParent;
}

void Dialog::setVisualParent(QQuickItem *visualParent)
{
    if (d->m_visualParent == visualParent) {
        return;
    }
    d->m_visualParent = visualParent;
    Q_EMIT visualParentChanged();
    if (visualParent) {
        if (visualParent->window()) {
            setTransientParent(visualParent->window());
        }
        d->syncToMainItemSize();
    }
}

QPoint Dialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if (!item) {
        // If no item was specified try to align at the center of the parent view
        auto *parentItem = qobject_cast<QQuickItem *>(parent());
        if (parentItem) {
            QScreen *screen = parentItem->window()->screen();

            switch (d->m_location) {
                case LingmoUIQuick::Dialog::PopupLocation::TopEdge:
                    return {screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().y()};
                case LingmoUIQuick::Dialog::PopupLocation::LeftEdge:
                    return {screen->availableGeometry().x(), screen->availableGeometry().center().y() - size.height() / 2};
                case LingmoUIQuick::Dialog::PopupLocation::RightEdge:
                    return {screen->availableGeometry().right() - size.width(), screen->availableGeometry().center().y() - size.height() / 2};
                case LingmoUIQuick::Dialog::PopupLocation::BottomEdge:
                    return {screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().bottom() - size.height()};
                    // Default center in the screen
                default:
                    return screen->geometry().center() - QPoint(size.width() / 2, size.height() / 2);
            }
        } else {
            return {};
        }
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));
    if (item->window()) {
        pos = item->window()->mapToGlobal(pos.toPoint());
    } else {
        return {};
    }

    // if the item is in a dock or in a window that ignores WM we want to position the popups outside of the dock
    const KWindowInfo winInfo(item->window()->winId(), NET::WMWindowType);
    const bool outsideParentWindow =
            ((winInfo.windowType(NET::AllTypesMask) == NET::Dock) || (item->window()->flags() & Qt::X11BypassWindowManagerHint)) && item->window()->mask().isNull();

    QRect parentGeometryBounds;
    if (outsideParentWindow) {
        parentGeometryBounds = item->window()->geometry();
    } else {
        parentGeometryBounds = item->mapRectToScene(item->boundingRect()).toRect();
        if (item->window()) {
            parentGeometryBounds.moveTopLeft(item->window()->mapToGlobal(parentGeometryBounds.topLeft()));
            pos = parentGeometryBounds.topLeft();
        }
    }

    const QPoint topPoint(pos.x() + (item->mapRectToScene(item->boundingRect()).width() - size.width()) / 2, parentGeometryBounds.top() - size.height());
    const QPoint bottomPoint(pos.x() + (item->mapRectToScene(item->boundingRect()).width() - size.width()) / 2, parentGeometryBounds.bottom());

    const QPoint leftPoint(parentGeometryBounds.left() - size.width(), pos.y() + (item->mapRectToScene(item->boundingRect()).height() - size.height()) / 2);

    const QPoint rightPoint(parentGeometryBounds.right(), pos.y() + (item->mapRectToScene(item->boundingRect()).height() - size.height()) / 2);

    QPoint dialogPos;
    if (d->m_location == LingmoUIQuick::Dialog::PopupLocation::TopEdge) {
        dialogPos = bottomPoint;
    } else if (d->m_location == LingmoUIQuick::Dialog::PopupLocation::LeftEdge) {
        dialogPos = rightPoint;
    } else if (d->m_location == LingmoUIQuick::Dialog::PopupLocation::RightEdge) {
        dialogPos = leftPoint;
    } else { // Types::BottomEdge
        dialogPos = topPoint;
    }

    // find the correct screen for the item
    // we do not rely on item->window()->screen() because
    // QWindow::screen() is always only the screen where the window gets first created
    // not actually the current window. See QWindow::screen() documentation
    QRect avail = item->window()->screen()->availableGeometry();

    if (outsideParentWindow) {
        // make the panel look it's inside the panel, in order to not make it look cut
        switch (d->m_location) {
            case LingmoUIQuick::Dialog::PopupLocation::LeftEdge:
            case LingmoUIQuick::Dialog::PopupLocation::RightEdge:
                avail.setTop(qMax(avail.top(), parentGeometryBounds.top()));
                avail.setBottom(qMin(avail.bottom(), parentGeometryBounds.bottom()));
                break;
            default:
                avail.setLeft(qMax(avail.left(), parentGeometryBounds.left()));
                avail.setRight(qMin(avail.right(), parentGeometryBounds.right()));
                break;
        }
    }

    if (dialogPos.x() < avail.left()) {
        // popup hits lhs
        if (d->m_location != LingmoUIQuick::Dialog::PopupLocation::LeftEdge || d->m_location == LingmoUIQuick::Dialog::PopupLocation::RightEdge) {
            // move it
            dialogPos.setX(avail.left());
        } else {
            // swap edge
            dialogPos.setX(rightPoint.x());
        }
    }
    if (dialogPos.x() + size.width() > avail.right()) {
        // popup hits rhs
        if (d->m_location == LingmoUIQuick::Dialog::PopupLocation::TopEdge || d->m_location == LingmoUIQuick::Dialog::PopupLocation::BottomEdge) {
            dialogPos.setX(qMax(avail.left(), (avail.right() - size.width() + 1)));
        } else {
            dialogPos.setX(leftPoint.x());
        }
    }
    if (dialogPos.y() < avail.top()) {
        // hitting top
        if (d->m_location == LingmoUIQuick::Dialog::PopupLocation::LeftEdge || d->m_location ==LingmoUIQuick::Dialog::PopupLocation::RightEdge) {
            dialogPos.setY(avail.top());
        } else {
            dialogPos.setY(bottomPoint.y());
        }
    }

    if (dialogPos.y() + size.height() > avail.bottom()) {
        // hitting bottom
        if (d->m_location == LingmoUIQuick::Dialog::PopupLocation::TopEdge || d->m_location == LingmoUIQuick::Dialog::PopupLocation::BottomEdge) {
            dialogPos.setY(topPoint.y());
        } else {
            dialogPos.setY(qMax(avail.top(), (avail.bottom() - size.height() + 1)));
        }
    }
    if(d->m_margin) {
        switch (d->m_location) {
            case TopEdge:
                dialogPos.setY(dialogPos.y() + d->m_margin);
                break;
            case BottomEdge:
                dialogPos.setY(dialogPos.y() - d->m_margin);
                break;
            case LeftEdge:
                dialogPos.setX(dialogPos.x() + d->m_margin);
                break;
            case RightEdge:
                dialogPos.setX(dialogPos.x() - d->m_margin);
                break;
            default:
                break;
        }
    }


    return dialogPos;
}

LingmoUIQuick::Dialog::PopupLocation Dialog::location() const
{
    return d->m_location;;
}

void Dialog::setLocation(LingmoUIQuick::Dialog::PopupLocation location)
{
    if (d->m_location == location) {
        return;
    }
    d->m_location = location;
    Q_EMIT locationChanged();

    if (d->m_mainItem) {
        d->syncToMainItemSize();
    }
}

void Dialog::adjustGeometry(const QRect &geom)
{
    if(d->m_windowProxy) {
        d->m_windowProxy->setGeometry(geom);
    }
}

bool Dialog::hideOnWindowDeactivate() const
{
    return d->m_hideOnWindowDeactivate;
}

void Dialog::setHideOnWindowDeactivate(bool hide)
{
    if (d->m_hideOnWindowDeactivate == hide) {
        return;
    }
    d->m_hideOnWindowDeactivate = hide;
    Q_EMIT hideOnWindowDeactivateChanged();
}

void Dialog::classBegin()
{
    d->m_componentComplete = false;
}

void Dialog::componentComplete()
{
    d->m_componentComplete = true;
    QQuickWindow::setVisible(d->m_visible);
}

void Dialog::resizeEvent(QResizeEvent *re)
{
    QQuickWindow::resizeEvent(re);

    // it's a spontaneous event generated in qguiapplication.cpp QGuiApplicationPrivate::processWindowScreenChangedEvent
    // QWindowSystemInterfacePrivate::GeometryChangeEvent gce(window, QHighDpi::fromNativePixels(window->handle()->geometry(), window), QRect());
    // This happens before the first show event when there is more than one screen,
    // right after the window has been created, the window is still 0x0,
    // but the resize event gets delivered with 0x0 again and executed with all the bad side effects
    // this seems to happen for every window when there are multiple screens, so something we have probably to watch out for in the future
    if (re->size().isEmpty() || re->size() == re->oldSize()) {
        return;
    }

    // A dialog can be resized even if no mainItem has ever been set
    if (!d->m_mainItem) {
        return;
    }

    d->m_mainItem->disconnect(this);
    d->m_mainItem->setPosition(QPointF(0, 0));
    d->m_mainItem->setSize(QSize(re->size().width(), re->size().height()));

    connect(d->m_mainItem, &QQuickItem::widthChanged, this, [&](){
       d->slotMainItemSizeChanged();
    });
    connect(d->m_mainItem, &QQuickItem::heightChanged, this, [&](){
        d->slotMainItemSizeChanged();
    });
}

void Dialog::focusOutEvent(QFocusEvent *ev)
{
    if (d->m_hideOnWindowDeactivate) {
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

        if (!parentHasFocus && !childHasFocus) {
            setVisible(false);
            Q_EMIT windowDeactivated();
        }
    }
    QQuickWindow::focusOutEvent(ev);
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        d->updateVisibility(true);
    }

    /*Fitt's law: if the containment has margins, and the mouse cursor clicked
     * on the mouse edge, forward the click in the containment boundaries
     */
    if (d->m_mainItem && !d->m_mainItem->size().isEmpty()) {
        switch (event->type()) {
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);

                // don't mess with position if the cursor is actually outside the view:
                // somebody is doing a click and drag that must not break when the cursor i outside
                if (geometry().contains(me->screenPos().toPoint()) && !d->mainItemContainsPosition(me->windowPos())) {
                    QMouseEvent me2(me->type(),
                                    d->positionAdjustedForMainItem(me->windowPos()),
                                    d->positionAdjustedForMainItem(me->windowPos()),
                                    d->positionAdjustedForMainItem(me->windowPos()) + position(),
                                    me->button(),
                                    me->buttons(),
                                    me->modifiers());

                    if (isVisible()) {
                        QCoreApplication::sendEvent(this, &me2);
                    }
                    return true;
                }
                break;
            }

            case QEvent::Wheel: {
                QWheelEvent *we = static_cast<QWheelEvent *>(event);

                const QPoint pos = we->position().toPoint();

                if (!d->mainItemContainsPosition(pos)) {
                    QWheelEvent we2(d->positionAdjustedForMainItem(pos),
                                    d->positionAdjustedForMainItem(pos) + position(),
                                    we->pixelDelta(),
                                    we->angleDelta(),
                                    we->buttons(),
                                    we->modifiers(),
                                    we->phase(),
                                    false /*inverted*/);

                    if (isVisible()) {
                        QCoreApplication::sendEvent(this, &we2);
                    }
                    return true;
                }
                break;
            }

            case QEvent::DragEnter: {
                QDragEnterEvent *de = static_cast<QDragEnterEvent *>(event);
                if (!d->mainItemContainsPosition(de->pos())) {
                    QDragEnterEvent de2(d->positionAdjustedForMainItem(de->pos()).toPoint(),
                                        de->possibleActions(),
                                        de->mimeData(),
                                        de->mouseButtons(),
                                        de->keyboardModifiers());

                    if (isVisible()) {
                        QCoreApplication::sendEvent(this, &de2);
                    }
                    return true;
                }
                break;
            }
                // DragLeave just works
            case QEvent::DragLeave:
                break;
            case QEvent::DragMove: {
                QDragMoveEvent *de = static_cast<QDragMoveEvent *>(event);
                if (!d->mainItemContainsPosition(de->pos())) {
                    QDragMoveEvent de2(d->positionAdjustedForMainItem(de->pos()).toPoint(),
                                       de->possibleActions(),
                                       de->mimeData(),
                                       de->mouseButtons(),
                                       de->keyboardModifiers());

                    if (isVisible()) {
                        QCoreApplication::sendEvent(this, &de2);
                    }
                    return true;
                }
                break;
            }
            case QEvent::Drop: {
                QDropEvent *de = static_cast<QDropEvent *>(event);
                if (!d->mainItemContainsPosition(de->pos())) {
                    QDropEvent de2(d->positionAdjustedForMainItem(de->pos()).toPoint(),
                                   de->possibleActions(),
                                   de->mimeData(),
                                   de->mouseButtons(),
                                   de->keyboardModifiers());

                    if (isVisible()) {
                        QCoreApplication::sendEvent(this, &de2);
                    }
                    return true;
                }
                break;
            }

            default:
                break;
        }
    }

    return QQuickWindow::event(event);
}

void Dialog::setType(LingmoUIQuick::WindowType::Type type)
{
    if (type == d->m_type) {
        return;
    }

    d->m_type = type;
    d->applyType();
    Q_EMIT typeChanged();
}

LingmoUIQuick::WindowType::Type Dialog::type() const
{
    return d->m_type;;
}

bool Dialog::isVisible() const
{
    if (d->m_componentComplete) {
        return QQuickWindow::isVisible();
    }
    return d->m_visible;
}

void Dialog::setVisible(bool visible)
{
    // only update real visibility when we have finished component completion
    // and all flags have been set
    d->m_visible = visible;
    if (d->m_componentComplete) {
        if (visible && d->m_visualParent && d->m_windowProxy) {
            d->m_windowProxy->setPosition(popupPosition(d->m_visualParent, size()));
        }

        // Bug 381242: Qt remembers minimize state and re-applies it when showing
        setWindowStates(windowStates() & ~Qt::WindowMinimized);
        QQuickWindow::setVisible(visible);
        // signal will be emitted and proxied from the QQuickWindow code
    } else {
        Q_EMIT visibleChangedProxy();
    }
}

bool Dialog::enableWindowBlur() const
{
    return d->m_enableWindowBlur;
}

void Dialog::setEnableWindowBlur(bool enable)
{
    if (d->m_enableWindowBlur == enable) {
        return;
    }

    d->m_enableWindowBlur = enable;
    d->updateBlurRegion();

    Q_EMIT enableWindowBlurChanged();
}

QRegion Dialog::blurRegion() const
{
    return d->m_windowBlurRegion;
}

void Dialog::setBlurRegion(const QRegion &region)
{
    if (d->m_windowBlurRegion == region) {
        return;
    }

    d->m_windowBlurRegion = region;
    d->updateBlurRegion();
}

int Dialog::margin() const
{
    return d->m_margin;
}

void Dialog::setMargin(int margin)
{
    if(d->m_margin != margin) {
        d->m_margin = margin;
        Q_EMIT marginChanged();
    }
}

bool Dialog::removeHeaderBar() const
{
    return d->m_removeHeaderBar;
}

void Dialog::setRemoveHeaderBar(bool remove)
{
    if(d->m_removeHeaderBar != remove) {
        d->m_removeHeaderBar = remove;
        if(d->m_windowProxy) {
            d->m_windowProxy->setRemoveTitleBar(remove);
        }
        Q_EMIT removeHeaderBar();
    }
}

void Dialog::updatePosition()
{
    d->syncToMainItemSize();
}
} // LingmoUIQuick

#include "moc_dialog.cpp"
