/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dialog.h"
#include "../declarativeimports/core/config-x11.h"
#include "appletquickitem.h"
#include "config-lingmo.h"
#include "configview.h"
#include "dialogbackground_p.h"
#include "dialogshadows_p.h"
#include "sharedqmlengine.h"

#include <QLayout>
#include <QMenu>
#include <QPlatformSurfaceEvent>
#include <QPointer>
#include <QQuickItem>
#include <QScreen>

#include <KWindowInfo>
#include <KWindowSystem>
#include <KX11Extras>

#include <KWindowEffects>
#include <Lingmo/Corona>

#include <QDebug>
#include <optional>

#if HAVE_X11
#include <qpa/qplatformwindow_p.h>
#endif

#include "lingmoshellwaylandintegration.h"

// Unfortunately QWINDOWSIZE_MAX is not exported
#define DIALOGSIZE_MAX ((1 << 24) - 1)

namespace LingmoQuick
{
class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
        : q(dialog)
        , location(Lingmo::Types::BottomEdge)
        , dialogBackground(new DialogBackground(q->contentItem()))
        , hasMask(false)
        , type(Dialog::Normal)
        , hideOnWindowDeactivate(false)
        , outputOnly(false)
        , visible(false)
        , resizableEdges({})
        , floating(0)
        , overridingCursor(false)
        , appletInterface(nullptr)
        , componentComplete(dialog->parent() == nullptr)
        , needsSetupNextExpose(true)
        , backgroundHints(Dialog::StandardBackground)
    {
    }

    // SLOTS
    /**
     * Sync Borders updates the enabled borders of the dialogBackground depending
     * on the geometry of the window.
     *
     * \param windowGeometry The window geometry which should be taken into
     * consideration when activating/deactivating certain borders
     */
    void syncBorders(const QRect &windowGeometry);

    /**
     * This function sets the blurBehind, background contrast and shadows. It
     * does so wrt the dialogBackground. So make sure the dialogBackground is the
     * correct size before calling this function.
     */
    void updateTheme();
    void updateVisibility(bool visible);

    void updateMinimumWidth();
    void updateMinimumHeight();
    void updateMaximumWidth();
    void updateMaximumHeight();
    void updateResizableEdges();
    void updateSizeFromAppletInterface();

    /**
     * Gets the maximum and minimum size hints for the window based on the contents. it doesn't actually resize anything
     */
    void getSizeHints(QSize &min, QSize &max) const;

    /**
     * This function is an optimized version of updateMaximumHeight,
     * updateMaximumWidth,updateMinimumWidth and updateMinimumHeight.
     * It should be called when you need to call all 4 of these functions
     * AND you have called syncToMainItemSize before.
     */
    void updateLayoutParameters();

    QRect availableScreenGeometryForPosition(const QPoint &pos) const;

    /**
     * This function checks the current position of the dialog and repositions
     * it so that no part of it is not on the screen
     */
    void repositionIfOffScreen();

    void slotMainItemSizeChanged();
    void slotWindowPositionChanged();

    void syncToMainItemSize();

    bool mainItemContainsPosition(const QPointF &point) const;
    QPointF positionAdjustedForMainItem(const QPointF &point) const;

    void applyType();

    bool updateMouseCursor(const QPointF &globalMousePos);
    Qt::Edges hitTest(const QPointF &pos);
    bool hitTestLeft(const QPointF &pos);
    bool hitTestRight(const QPointF &pos);
    bool hitTestTop(const QPointF &pos);
    bool hitTestBottom(const QPointF &pos);

    Dialog *q;
    Lingmo::Types::Location location;
    DialogBackground *dialogBackground;
    QPointer<QQuickItem> mainItem;
    QPointer<QQuickItem> visualParent;

    QRect cachedGeometry;
    bool hasMask;
    Dialog::WindowType type;
    bool hideOnWindowDeactivate;
    bool outputOnly;
    bool visible;
    Qt::Edges resizableEdges;
    int floating;
    bool overridingCursor;
    AppletQuickItem *appletInterface;
    Lingmo::Theme theme;
    bool componentComplete;
    bool needsSetupNextExpose;
    Dialog::BackgroundHints backgroundHints;

    // Attached Layout property of mainItem, if any
    QPointer<QObject> mainItemLayout;
};

static bool isRunningInKWin()
{
    static bool check = QGuiApplication::platformName() == QLatin1String("wayland-org.kde.kwin.qpa");
    return check;
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

void DialogPrivate::syncBorders(const QRect &geom)
{
    QRect avail = availableScreenGeometryForPosition(geom.topLeft());
    int borders = KSvg::FrameSvg::AllBorders;

    // Tooltips always have all the borders
    // floating windows have all borders
    if (!q->flags().testFlag(Qt::ToolTip) && location != Lingmo::Types::Floating && floating == 0) {
        if (geom.x() <= avail.x() || location == Lingmo::Types::LeftEdge) {
            borders = borders & ~KSvg::FrameSvg::LeftBorder;
        }
        if (geom.y() <= avail.y() || location == Lingmo::Types::TopEdge) {
            borders = borders & ~KSvg::FrameSvg::TopBorder;
        }
        if (avail.right() <= geom.x() + geom.width() || location == Lingmo::Types::RightEdge) {
            borders = borders & ~KSvg::FrameSvg::RightBorder;
        }
        if (avail.bottom() <= geom.y() + geom.height() || location == Lingmo::Types::BottomEdge) {
            borders = borders & ~KSvg::FrameSvg::BottomBorder;
        }
    }

    if (dialogBackground->enabledBorders() != (KSvg::FrameSvg::EnabledBorder)borders) {
        dialogBackground->setEnabledBorders((KSvg::FrameSvg::EnabledBorder)borders);
    }
}

void DialogPrivate::updateTheme()
{
    if (backgroundHints == Dialog::NoBackground) {
        dialogBackground->setImagePath(QString());
        KWindowEffects::enableBlurBehind(q, false);
        KWindowEffects::enableBackgroundContrast(q, false);
        q->setMask(QRegion());
        DialogShadows::instance()->removeWindow(q);
    } else {
        auto prefix = QStringLiteral("");
        if ((backgroundHints & Dialog::SolidBackground) == Dialog::SolidBackground) {
            prefix = QStringLiteral("solid/");
        }
        if (type == Dialog::Tooltip) {
            dialogBackground->setImagePath(prefix + QStringLiteral("widgets/tooltip"));
        } else {
            dialogBackground->setImagePath(prefix + QStringLiteral("dialogs/background"));
        }

        const QRegion mask = dialogBackground->mask();
        KWindowEffects::enableBlurBehind(q, theme.blurBehindEnabled(), mask);

        KWindowEffects::enableBackgroundContrast(q,
                                                 theme.backgroundContrastEnabled(),
                                                 theme.backgroundContrast(),
                                                 theme.backgroundIntensity(),
                                                 theme.backgroundSaturation(),
                                                 mask);

        if (!KWindowSystem::isPlatformX11() || KX11Extras::compositingActive()) {
            if (hasMask) {
                hasMask = false;
                q->setMask(QRegion());
            }
        } else {
            hasMask = true;
            q->setMask(dialogBackground->mask());
        }
        if (q->isVisible()) {
            DialogShadows::instance()->addWindow(q, dialogBackground->enabledBorders());
        }
    }
}

void DialogPrivate::updateVisibility(bool visible)
{
    if (visible) {
        if (visualParent && visualParent->window()) {
            q->setTransientParent(visualParent->window());
        }

        if (q->location() == Lingmo::Types::FullScreen) {
            dialogBackground->setEnabledBorders(KSvg::FrameSvg::NoBorder);

            // We cache the original size of the item, to retrieve it
            // when the dialog is switched back from fullscreen.
            if (q->geometry() != q->screen()->availableGeometry()) {
                cachedGeometry = q->geometry();
            }
            q->setGeometry(q->screen()->availableGeometry());
        } else {
            if (!cachedGeometry.isNull()) {
                q->resize(cachedGeometry.size());
                slotWindowPositionChanged();
                if (visualParent) {
                    q->setPosition(q->popupPosition(visualParent, q->size()));
                }
                cachedGeometry = QRect();
            }

            if (mainItem) {
                syncToMainItemSize();
            }
            if (mainItemLayout) {
                updateLayoutParameters();
            }

            // if is a wayland window that was hidden, we need
            // to set its position again as there won't be any move event to sync QWindow::position and shellsurface::position
            if (type != Dialog::OnScreenDisplay) {
                LingmoShellWaylandIntegration::get(q)->setPosition(q->position());
            }
        }
    }

    if (!q->flags().testFlag(Qt::ToolTip) && type != Dialog::Notification && type != Dialog::CriticalNotification) {
        KWindowEffects::SlideFromLocation slideLocation = KWindowEffects::NoEdge;

        switch (location) {
        case Lingmo::Types::TopEdge:
            slideLocation = KWindowEffects::TopEdge;
            break;
        case Lingmo::Types::LeftEdge:
            slideLocation = KWindowEffects::LeftEdge;
            break;
        case Lingmo::Types::RightEdge:
            slideLocation = KWindowEffects::RightEdge;
            break;
        case Lingmo::Types::BottomEdge:
            slideLocation = KWindowEffects::BottomEdge;
            break;
        // no edge, no slide
        default:
            break;
        }

        KWindowEffects::slideWindow(q, slideLocation, -1);
    }

    if (visible) {
        q->raise();

        applyType();
    }
}

void DialogPrivate::updateMinimumWidth()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMinimumWidth(0);

    // this is to try to get the internal item resized a tad before, but
    // the flicker almost always happen anyways, so is *probably* useless
    // this other kind of flicker is the view not being always focused exactly
    // on the scene
    int minimumWidth = mainItemLayout->property("minimumWidth").toInt() + dialogBackground->leftMargin() + dialogBackground->rightMargin();
    if (q->screen()) {
        minimumWidth = qMin(q->screen()->availableGeometry().width(), minimumWidth);
    }
    q->contentItem()->setWidth(qMax(q->width(), minimumWidth));
    q->setWidth(qMax(q->width(), minimumWidth));

    updateLayoutParameters();
}

void DialogPrivate::updateMinimumHeight()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMinimumHeight(0);

    // this is to try to get the internal item resized a tad before, but
    // the flicker almost always happen anyways, so is *probably* useless
    // this other kind of flicker is the view not being always focused exactly
    // on the scene
    int minimumHeight = mainItemLayout->property("minimumHeight").toInt() + dialogBackground->topMargin() + dialogBackground->bottomMargin();
    if (q->screen()) {
        minimumHeight = qMin(q->screen()->availableGeometry().height(), minimumHeight);
    }
    q->contentItem()->setHeight(qMax(q->height(), minimumHeight));
    q->setHeight(qMax(q->height(), minimumHeight));

    updateLayoutParameters();
}

void DialogPrivate::updateMaximumWidth()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMaximumWidth(DIALOGSIZE_MAX);

    int maximumWidth = mainItemLayout->property("maximumWidth").toInt() + dialogBackground->leftMargin() + dialogBackground->rightMargin();
    if (q->screen()) {
        maximumWidth = qMin(q->screen()->availableGeometry().width(), maximumWidth);
    }
    q->contentItem()->setWidth(qMin(q->width(), maximumWidth));
    q->setWidth(qMin(q->width(), maximumWidth));

    updateLayoutParameters();
}

void DialogPrivate::updateMaximumHeight()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMaximumHeight(DIALOGSIZE_MAX);

    int maximumHeight = mainItemLayout->property("maximumHeight").toInt() + dialogBackground->topMargin() + dialogBackground->bottomMargin();
    if (q->screen()) {
        maximumHeight = qMin(q->screen()->availableGeometry().height(), maximumHeight);
    }
    q->contentItem()->setHeight(qMin(q->height(), maximumHeight));
    q->setHeight(qMin(q->height(), maximumHeight));

    updateLayoutParameters();
}

void DialogPrivate::updateResizableEdges()
{
    if (!appletInterface) {
        resizableEdges = {};
        return;
    }

    QSize min;
    QSize max(DIALOGSIZE_MAX, DIALOGSIZE_MAX);
    getSizeHints(min, max);
    if (min == max) {
        resizableEdges = {};
        return;
    }

    switch (q->location()) {
    case Lingmo::Types::BottomEdge:
        resizableEdges = Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge;
        break;
    case Lingmo::Types::TopEdge:
        resizableEdges = Qt::LeftEdge | Qt::BottomEdge | Qt::RightEdge;
        break;
    case Lingmo::Types::LeftEdge:
        resizableEdges = Qt::TopEdge | Qt::BottomEdge | Qt::RightEdge;
        break;
    case Lingmo::Types::RightEdge:
        resizableEdges = Qt::LeftEdge | Qt::TopEdge | Qt::BottomEdge;
        break;
    case Lingmo::Types::Floating:
    case Lingmo::Types::Desktop:
    case Lingmo::Types::FullScreen:
        resizableEdges = {};
        break;
    }
}

void DialogPrivate::updateSizeFromAppletInterface()
{
    if (!appletInterface) {
        return;
    }
    if (!mainItem) {
        return;
    }
    if (!mainItemLayout) {
        return;
    }

    QSize min;
    QSize max(DIALOGSIZE_MAX, DIALOGSIZE_MAX);
    getSizeHints(min, max);
    if (min == max) {
        return;
    }

    QVariant prefHeight = mainItemLayout->property("preferredHeight");
    QVariant prefWidth = mainItemLayout->property("preferredWidth");
    int defHeight = prefHeight.isNull() ? min.height() : prefHeight.toInt();
    int defWidth = prefWidth.isNull() ? min.width() : prefWidth.toInt();

    KConfigGroup config = appletInterface->applet()->config();
    qreal popupWidth = config.readEntry("popupWidth", static_cast<qreal>(defWidth));
    qreal popupHeight = config.readEntry("popupHeight", static_cast<qreal>(defHeight));
    mainItemLayout->setProperty("preferredWidth", popupWidth);
    mainItemLayout->setProperty("preferredHeight", popupHeight);
    mainItem->setWidth(popupWidth);
    mainItem->setHeight(popupHeight);
    updateLayoutParameters();
}

void DialogPrivate::getSizeHints(QSize &min, QSize &max) const
{
    if (!componentComplete || !mainItem || !mainItemLayout) {
        return;
    }
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    int minimumHeight = mainItemLayout->property("minimumHeight").toInt();
    int maximumHeight = mainItemLayout->property("maximumHeight").toInt();
    maximumHeight = maximumHeight > 0 ? qMax(minimumHeight, maximumHeight) : DIALOGSIZE_MAX;

    int minimumWidth = mainItemLayout->property("minimumWidth").toInt();
    int maximumWidth = mainItemLayout->property("maximumWidth").toInt();
    maximumWidth = maximumWidth > 0 ? qMax(minimumWidth, maximumWidth) : DIALOGSIZE_MAX;

    minimumHeight += dialogBackground->topMargin() + dialogBackground->bottomMargin();
    maximumHeight += dialogBackground->topMargin() + dialogBackground->bottomMargin();
    minimumWidth += dialogBackground->leftMargin() + dialogBackground->rightMargin();
    maximumWidth += dialogBackground->leftMargin() + dialogBackground->rightMargin();

    if (q->screen()) {
        minimumWidth = qMin(q->screen()->availableGeometry().width(), minimumWidth);
        minimumHeight = qMin(q->screen()->availableGeometry().height(), minimumHeight);
        maximumWidth = qMin(q->screen()->availableGeometry().width(), maximumWidth);
        maximumHeight = qMin(q->screen()->availableGeometry().height(), maximumHeight);
    }

    // Make sure that we never return min that would be larger than max
    min = QSize(qMin(minimumWidth, maximumWidth), qMin(minimumHeight, maximumHeight));
    max = QSize(maximumWidth, maximumHeight);
}

void DialogPrivate::updateLayoutParameters()
{
    if (!componentComplete || !mainItem || !mainItemLayout || q->visibility() == QWindow::Hidden) {
        return;
    }

    mainItem->disconnect(q);

    QSize min;
    QSize max(DIALOGSIZE_MAX, DIALOGSIZE_MAX);
    getSizeHints(min, max);

    const QSize finalSize(qBound(min.width(), q->width(), std::max(max.width(), min.width())),
                          qBound(min.height(), q->height(), std::max(max.height(), min.height())));

    if (visualParent) {
        // it's important here that we're using re->size() as size, we don't want to do recursive resizeEvents
        const QRect geom(q->popupPosition(visualParent, finalSize), finalSize);
        q->adjustGeometry(geom);
    } else {
        q->resize(finalSize);
    }

    mainItem->setPosition(QPointF(dialogBackground->leftMargin(), dialogBackground->topMargin()));
    mainItem->setSize(QSizeF(q->width() - dialogBackground->leftMargin() - dialogBackground->rightMargin(),
                             q->height() - dialogBackground->topMargin() - dialogBackground->bottomMargin()));

    dialogBackground->setSize(QSizeF(q->width(), q->height()));

    if (!needsSetupNextExpose && visible) {
        // Only reposition after successful setup; otherwise repositionIfOffScreen will override the default position set by kwin under wayland
        repositionIfOffScreen();
    }
    updateTheme();

    // setting the minimum or maximum size will resize the window instantly and min <= max is enforced
    // so we have to set maximum first in that case, but also care about the new maximum being smaller
    // than the current minimum
    // QTBUG-113233
    q->setMaximumSize(max.expandedTo(q->maximumSize()));
    q->setMinimumSize(min);
    q->setMaximumSize(max);

    QObject::connect(mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::repositionIfOffScreen()
{
    if (!componentComplete) {
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

    q->setX(x);
    q->setY(y);
}

void DialogPrivate::syncToMainItemSize()
{
    Q_ASSERT(mainItem);

    if (!componentComplete || q->visibility() == QWindow::Hidden) {
        return;
    }
    if (mainItem->width() <= 0 || mainItem->height() <= 0) {
        qWarning() << "trying to show an empty dialog";
    }

    updateTheme();
    if (visualParent) {
        const QSize fullSize = QSize(mainItem->width(), mainItem->height())
            + QSize(dialogBackground->leftMargin() + dialogBackground->rightMargin(), dialogBackground->topMargin() + dialogBackground->bottomMargin());

        // We get the popup position with the fullsize as we need the popup
        // position in order to determine our actual size, as the position
        // determines which borders will be shown.
        const QRect geom(q->popupPosition(visualParent, fullSize), fullSize);

        // We're then moving the window to where we think we would be with all
        // the borders. This way when syncBorders is called, it has a geometry
        // to work with.
        syncBorders(geom);
    } else {
        syncBorders(q->geometry());
    }

    QSize s = QSize(mainItem->width(), mainItem->height())
        + QSize(dialogBackground->leftMargin() + dialogBackground->rightMargin(), dialogBackground->topMargin() + dialogBackground->bottomMargin());

    QSize min;
    QSize max(DIALOGSIZE_MAX, DIALOGSIZE_MAX);
    getSizeHints(min, max);
    s = QSize(qBound(min.width(), s.width(), max.width()), qBound(min.height(), s.height(), max.height()));

    q->contentItem()->setSize(s);

    dialogBackground->setSize(s);

    if (visualParent) {
        const QRect geom(q->popupPosition(visualParent, s), s);

        if (geom == q->geometry()) {
            return;
        }

        q->adjustGeometry(geom);
        // The borders will instantly be updated but the geometry might take a
        // while as sub-classes can reimplement adjustGeometry and animate it.
        syncBorders(geom);

    } else {
        q->resize(s);
    }

    mainItem->setPosition(QPointF(dialogBackground->leftMargin(), dialogBackground->topMargin()));

    updateTheme();
}

void DialogPrivate::slotWindowPositionChanged()
{
    // Tooltips always have all the borders
    // floating windows have all borders
    if (!q->isVisible() || q->flags().testFlag(Qt::ToolTip) || location == Lingmo::Types::Floating || floating > 0) {
        return;
    }

    syncBorders(q->geometry());
    updateTheme();

    if (mainItem) {
        mainItem->setPosition(QPoint(dialogBackground->leftMargin(), dialogBackground->topMargin()));
        mainItem->setSize(QSize(q->width() - dialogBackground->leftMargin() - dialogBackground->rightMargin(),
                                q->height() - dialogBackground->topMargin() - dialogBackground->bottomMargin()));
    }
}

bool DialogPrivate::mainItemContainsPosition(const QPointF &point) const
{
    if (!mainItem) {
        return false;
    }

    return QRectF(mainItem->mapToScene(QPoint(0, 0)), QSizeF(mainItem->width(), mainItem->height())).contains(point);
}

QPointF DialogPrivate::positionAdjustedForMainItem(const QPointF &point) const
{
    if (!mainItem) {
        return point;
    }

    QRectF itemRect(mainItem->mapToScene(QPoint(0, 0)), QSizeF(mainItem->width(), mainItem->height()));

    return QPointF(qBound(itemRect.left(), point.x(), itemRect.right()), qBound(itemRect.top(), point.y(), itemRect.bottom()));
}

void DialogPrivate::applyType()
{
    /*QXcbWindowFunctions::WmWindowType*/ int wmType = 0;

#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        switch (type) {
        case Dialog::Normal:
            q->setFlags(Qt::FramelessWindowHint | q->flags());
            break;
        case Dialog::Dock:
            wmType = QNativeInterface::Private::QXcbWindow::Dock;
            break;
        case Dialog::DialogWindow:
            wmType = QNativeInterface::Private::QXcbWindow::Dialog;
            break;
        case Dialog::PopupMenu:
            wmType = QNativeInterface::Private::QXcbWindow::PopupMenu;
            break;
        case Dialog::Tooltip:
            wmType = QNativeInterface::Private::QXcbWindow::Tooltip;
            break;
        case Dialog::Notification:
            wmType = QNativeInterface::Private::QXcbWindow::Notification;
            break;
        case Dialog::OnScreenDisplay:
        case Dialog::CriticalNotification:
        case Dialog::AppletPopup:
            // Not supported by Qt
            break;
        }

        if (wmType) {
            // QXcbWindow isn't installed and thus inaccessible to us, but it does read this magic property from the window...
            q->setProperty("_q_xcb_wm_window_type", wmType);
        }
    }
#endif

    if (!wmType && type != Dialog::Normal && KWindowSystem::isPlatformX11()) {
        KX11Extras::setType(q->winId(), static_cast<NET::WindowType>(type));
    }
    if (q->flags() & Qt::WindowStaysOnTopHint) {
        // If the AppletPopup type is not explicitly requested, then use the Dock type in this case
        // to avoid bug #454635.
        if (type != Dialog::AppletPopup && type != Dialog::Tooltip) {
            type = Dialog::Dock;
            LingmoShellWaylandIntegration::get(q)->setPanelBehavior(QtWayland::org_kde_lingmo_surface::panel_behavior_windows_go_below);
        } else {
            LingmoShellWaylandIntegration::get(q)->setPanelBehavior(QtWayland::org_kde_lingmo_surface::panel_behavior_always_visible);
        }
    }
    switch (type) {
    case Dialog::Dock:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_panel);
        break;
    case Dialog::Tooltip:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_tooltip);
        break;
    case Dialog::Notification:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_notification);
        break;
    case Dialog::OnScreenDisplay:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_onscreendisplay);
        break;
    case Dialog::CriticalNotification:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_criticalnotification);
        break;
    case Dialog::Normal:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_normal);
        break;
    case Dialog::AppletPopup:
        LingmoShellWaylandIntegration::get(q)->setRole(QtWayland::org_kde_lingmo_surface::role_appletpopup);
        break;
    default:
        break;
    }

    // an OSD can't be a Dialog, as qt xcb would attempt to set a transient parent for it
    // see bug 370433
    if (type == Dialog::OnScreenDisplay) {
        Qt::WindowFlags flags = (q->flags() & ~Qt::Dialog) | Qt::Window;
        if (outputOnly) {
            flags |= Qt::WindowTransparentForInput;
        } else {
            flags &= ~Qt::WindowTransparentForInput;
        }
        q->setFlags(flags);
    }

    if (backgroundHints == Dialog::NoBackground) {
        dialogBackground->setImagePath(QString());
    } else {
        auto prefix = QStringLiteral("");
        if ((backgroundHints & Dialog::SolidBackground) == Dialog::SolidBackground) {
            prefix = QStringLiteral("solid/");
        }
        if (type == Dialog::Tooltip) {
            dialogBackground->setImagePath(prefix + QStringLiteral("widgets/tooltip"));
        } else {
            dialogBackground->setImagePath(prefix + QStringLiteral("dialogs/background"));
        }
    }

    if (KWindowSystem::isPlatformX11()) {
        if (type == Dialog::Dock || type == Dialog::Notification || type == Dialog::OnScreenDisplay || type == Dialog::CriticalNotification) {
            KX11Extras::setOnAllDesktops(q->winId(), true);
        } else {
            KX11Extras::setOnAllDesktops(q->winId(), false);
        }
    }

    LingmoShellWaylandIntegration::get(q)->setTakesFocus(!q->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
}

bool DialogPrivate::updateMouseCursor(const QPointF &globalMousePos)
{
    Qt::Edges sides = hitTest(globalMousePos) & resizableEdges;
    if (!sides) {
        if (overridingCursor) {
            q->unsetCursor();
            overridingCursor = false;
        }
        return false;
    }

    if (sides == Qt::Edges(Qt::LeftEdge | Qt::TopEdge)) {
        q->setCursor(Qt::SizeFDiagCursor);
    } else if (sides == Qt::Edges(Qt::RightEdge | Qt::TopEdge)) {
        q->setCursor(Qt::SizeBDiagCursor);
    } else if (sides == Qt::Edges(Qt::LeftEdge | Qt::BottomEdge)) {
        q->setCursor(Qt::SizeBDiagCursor);
    } else if (sides == Qt::Edges(Qt::RightEdge | Qt::BottomEdge)) {
        q->setCursor(Qt::SizeFDiagCursor);
    } else if (sides.testFlag(Qt::TopEdge)) {
        q->setCursor(Qt::SizeVerCursor);
    } else if (sides.testFlag(Qt::LeftEdge)) {
        q->setCursor(Qt::SizeHorCursor);
    } else if (sides.testFlag(Qt::RightEdge)) {
        q->setCursor(Qt::SizeHorCursor);
    } else {
        q->setCursor(Qt::SizeVerCursor);
    }

    overridingCursor = true;
    return true;
}

Qt::Edges DialogPrivate::hitTest(const QPointF &pos)
{
    bool left = hitTestLeft(pos);
    bool right = hitTestRight(pos);
    bool top = hitTestTop(pos);
    bool bottom = hitTestBottom(pos);
    Qt::Edges edges;
    if (left) {
        edges.setFlag(Qt::LeftEdge);
    }
    if (right) {
        edges.setFlag(Qt::RightEdge);
    }
    if (bottom) {
        edges.setFlag(Qt::BottomEdge);
    }
    if (top) {
        edges.setFlag(Qt::TopEdge);
    }

    return edges;
}

bool DialogPrivate::hitTestLeft(const QPointF &pos)
{
    const QRect geometry = q->geometry();
    const QRectF rect(geometry.x(), geometry.y(), dialogBackground->leftMargin(), geometry.height());
    return rect.contains(pos);
}

bool DialogPrivate::hitTestRight(const QPointF &pos)
{
    const QRect geometry = q->geometry();
    const QRectF rect(geometry.x() + geometry.width() - dialogBackground->rightMargin(), geometry.y(), dialogBackground->rightMargin(), geometry.height());
    return rect.contains(pos);
}

bool DialogPrivate::hitTestTop(const QPointF &pos)
{
    const QRect geometry = q->geometry();
    const QRectF rect(geometry.x(), geometry.y(), geometry.width(), dialogBackground->topMargin());
    return rect.contains(pos);
}

bool DialogPrivate::hitTestBottom(const QPointF &pos)
{
    const QRect geometry = q->geometry();
    const QRectF rect(geometry.x(), geometry.y() + geometry.height() - dialogBackground->bottomMargin(), geometry.width(), dialogBackground->bottomMargin());
    return rect.contains(pos);
}

Dialog::Dialog(QQuickItem *parent)
    : QQuickWindow(parent ? parent->window() : nullptr)
    , d(new DialogPrivate(this))
{
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint | Qt::Dialog);

    connect(this, &QWindow::xChanged, [this]() {
        d->slotWindowPositionChanged();
    });
    connect(this, &QWindow::yChanged, [this]() {
        d->slotWindowPositionChanged();
    });
    connect(this, &Dialog::locationChanged, this, [&] {
        d->updateResizableEdges();
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

    // This is needed as a transition thing for KWayland
    // FIXME: is this valid anymore?
    // setProperty("__lingmo_frameSvg", QVariant::fromValue(d->dialogBackground->frameSvg()));

    connect(&d->theme, SIGNAL(themeChanged()), this, SLOT(updateTheme()));
}

Dialog::~Dialog()
{
    // Prevent signals from super-class destructor invoking our now-destroyed slots
    disconnect(this, nullptr, this, nullptr);
}

QQuickItem *Dialog::mainItem() const
{
    return d->mainItem;
}

void Dialog::setMainItem(QQuickItem *mainItem)
{
    if (d->mainItem != mainItem) {
        if (d->mainItem) {
            disconnect(d->mainItem, nullptr, this, nullptr);
            d->mainItem->setParentItem(nullptr);
        }

        if (d->mainItemLayout) {
            disconnect(d->mainItemLayout, nullptr, this, nullptr);
        }

        d->mainItem = mainItem;

        if (mainItem) {
            mainItem->setParentItem(contentItem());

            connect(mainItem, SIGNAL(widthChanged()), this, SLOT(slotMainItemSizeChanged()));
            connect(mainItem, SIGNAL(heightChanged()), this, SLOT(slotMainItemSizeChanged()));
            d->slotMainItemSizeChanged();

            // Extract the representation's Layout, if any
            QObject *layout = nullptr;
            setMinimumSize(QSize(0, 0));
            setMaximumSize(QSize(DIALOGSIZE_MAX, DIALOGSIZE_MAX));

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

            d->mainItemLayout = layout;

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

void DialogPrivate::slotMainItemSizeChanged()
{
    syncToMainItemSize();
}

QQuickItem *Dialog::visualParent() const
{
    return d->visualParent;
}

void Dialog::setVisualParent(QQuickItem *visualParent)
{
    if (d->visualParent == visualParent) {
        return;
    }

    d->visualParent = visualParent;
    Q_EMIT visualParentChanged();
    if (visualParent) {
        if (visualParent->window()) {
            setTransientParent(visualParent->window());
        }
        if (d->mainItem) {
            d->syncToMainItemSize();
        }
    }
}

QPoint Dialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if (!item) {
        // If no item was specified try to align at the center of the parent view
        QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
        if (parentItem) {
            QScreen *screen = parentItem->window()->screen();

            switch (d->location) {
            case Lingmo::Types::TopEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().y());
            case Lingmo::Types::LeftEdge:
                return QPoint(screen->availableGeometry().x(), screen->availableGeometry().center().y() - size.height() / 2);
            case Lingmo::Types::RightEdge:
                return QPoint(screen->availableGeometry().right() - size.width(), screen->availableGeometry().center().y() - size.height() / 2);
            case Lingmo::Types::BottomEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().bottom() - size.height());
            // Default center in the screen
            default:
                return screen->geometry().center() - QPoint(size.width() / 2, size.height() / 2);
            }
        } else {
            return QPoint();
        }
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));

    if (item->window()) {
        pos = item->window()->mapToGlobal(pos.toPoint());
    } else {
        return QPoint();
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

    const QRectF itemSceneBoundingRect = item->mapRectToScene(item->boundingRect());
    const QPoint centerPoint(pos.x() + (itemSceneBoundingRect.width() - size.width()) / 2, //
                             pos.y() + (itemSceneBoundingRect.height() - size.height()) / 2);

    const QPoint topPoint(centerPoint.x(), parentGeometryBounds.top() - size.height());
    const QPoint bottomPoint(centerPoint.x(), parentGeometryBounds.bottom());

    const QPoint leftPoint(parentGeometryBounds.left() - size.width(), centerPoint.y());
    const QPoint rightPoint(parentGeometryBounds.right(), centerPoint.y());

    QPoint dialogPos;
    if (d->location == Lingmo::Types::TopEdge) {
        dialogPos = bottomPoint;
    } else if (d->location == Lingmo::Types::LeftEdge) {
        dialogPos = rightPoint;
    } else if (d->location == Lingmo::Types::RightEdge) {
        dialogPos = leftPoint;
    } else { // Types::BottomEdge
        dialogPos = topPoint;
    }

    // find the correct screen for the item
    // we do not rely on item->window()->screen() because
    // QWindow::screen() is always only the screen where the window gets first created
    // not actually the current window. See QWindow::screen() documentation
    QRect avail = item->window()->screen()->availableGeometry();
    avail.adjust(d->floating, d->floating, -d->floating, -d->floating);

    if (outsideParentWindow && d->dialogBackground->enabledBorders() != KSvg::FrameSvg::AllBorders) {
        // make the panel look it's inside the panel, in order to not make it look cut
        switch (d->location) {
        case Lingmo::Types::LeftEdge:
        case Lingmo::Types::RightEdge:
            avail.setTop(qMax(avail.top(), parentGeometryBounds.top()));
            avail.setBottom(qMin(avail.bottom(), parentGeometryBounds.bottom()));
            break;
        default:
            avail.setLeft(qMax(avail.left(), parentGeometryBounds.left()));
            avail.setRight(qMin(avail.right(), parentGeometryBounds.right()));
            break;
        }
    }

    // If the dialog is from opening an applet in the panel and it's close enough to the center that
    // it would still cover the original applet in the panel if it was centered, then we manually center it.
    if (d->type == Dialog::AppletPopup) {
        QRectF parentRect = item->mapRectToScene(item->boundingRect());
        switch (d->location) {
        case Lingmo::Types::TopEdge:
        case Lingmo::Types::BottomEdge:
            if (qAbs(dialogPos.x() + size.width() / 2 - avail.center().x()) < size.width() / 2 - parentRect.width() / 3) {
                dialogPos.setX(avail.center().x() - size.width() / 2);
            }
            break;
        case Lingmo::Types::LeftEdge:
        case Lingmo::Types::RightEdge:
            if (qAbs(dialogPos.y() + size.height() / 2 - avail.center().y()) < size.height() / 2 - parentRect.height() / 3) {
                dialogPos.setY(avail.center().y() - size.height() / 2);
            }
            break;
        default:
            break;
        }
    }

    // For top & bottom the inner conditions are intentionally different from thouse for left & right,
    // because we want floating popups to flip vertically, but only push them in bounds horizontally.

    // If popup goes out of bounds...
    // ...at the left edge
    if (dialogPos.x() < avail.left()) {
        if (d->location != Lingmo::Types::LeftEdge) {
            // move it in bounds
            // Note: floating popup goes here.
            dialogPos.setX(avail.left());
        } else {
            // flip it around
            dialogPos.setX(rightPoint.x());
        }
    }
    // ...at the right edge
    if (dialogPos.x() + size.width() > avail.right()) {
        if (d->location != Lingmo::Types::RightEdge) {
            // move it in bounds
            // Note: floating popup goes here.
            dialogPos.setX(qMax(avail.left(), (avail.right() - size.width() + 1)));
        } else {
            // flip it around
            dialogPos.setX(leftPoint.x());
        }
    }
    // ...at the top edge
    if (dialogPos.y() < avail.top()) {
        if (d->location == Lingmo::Types::LeftEdge || d->location == Lingmo::Types::RightEdge) {
            // move it in bounds
            dialogPos.setY(avail.top());
        } else {
            // flip it around
            // Note: floating popup goes here.
            dialogPos.setY(bottomPoint.y());
        }
    }
    // ...at the bottom edge
    if (dialogPos.y() + size.height() > avail.bottom()) {
        if (d->location == Lingmo::Types::LeftEdge || d->location == Lingmo::Types::RightEdge) {
            // move it in bounds
            dialogPos.setY(qMax(avail.top(), (avail.bottom() - size.height() + 1)));
        } else {
            // flip it around
            // Note: floating popup goes here.
            dialogPos.setY(topPoint.y());
        }
    }

    return dialogPos;
}

Lingmo::Types::Location Dialog::location() const
{
    return d->location;
}

void Dialog::setLocation(Lingmo::Types::Location location)
{
    if (d->location == location) {
        return;
    }
    d->location = location;
    Q_EMIT locationChanged();

    if (d->mainItem) {
        d->syncToMainItemSize();
    }
}

QObject *Dialog::margins() const
{
    return d->dialogBackground->fixedMargins();
}

QObject *Dialog::inset() const
{
    return d->dialogBackground->inset();
}

void Dialog::setFramelessFlags(Qt::WindowFlags flags)
{
    if (d->type == Dialog::Normal) {
        flags |= Qt::Dialog;
    }
    setFlags(Qt::FramelessWindowHint | flags);
    d->applyType();
    Q_EMIT flagsChanged();
}

void Dialog::adjustGeometry(const QRect &geom)
{
    setGeometry(geom);
}

void Dialog::resizeEvent(QResizeEvent *re)
{
    QQuickWindow::resizeEvent(re);

    if (d->resizableEdges) {
        d->updateMouseCursor(QCursor::pos());
    }

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
    if (!d->mainItem || !isExposed()) {
        return;
    }

    d->mainItem->disconnect(this);

    d->dialogBackground->setSize(QSizeF(re->size().width(), re->size().height()));
    d->mainItem->setPosition(QPointF(d->dialogBackground->leftMargin(), d->dialogBackground->topMargin()));

    d->mainItem->setSize(QSize(re->size().width() - d->dialogBackground->leftMargin() - d->dialogBackground->rightMargin(),
                               re->size().height() - d->dialogBackground->topMargin() - d->dialogBackground->bottomMargin()));

    d->updateTheme();

    QObject::connect(d->mainItem, SIGNAL(widthChanged()), this, SLOT(slotMainItemSizeChanged()));
    QObject::connect(d->mainItem, SIGNAL(heightChanged()), this, SLOT(slotMainItemSizeChanged()));
}

void Dialog::setType(WindowType type)
{
    if (type == d->type) {
        return;
    }

    d->type = type;
    d->applyType();
    Q_EMIT typeChanged();
}

Dialog::WindowType Dialog::type() const
{
    return d->type;
}

void Dialog::focusInEvent(QFocusEvent *ev)
{
    QQuickWindow::focusInEvent(ev);
}

void Dialog::focusOutEvent(QFocusEvent *ev)
{
    if (d->hideOnWindowDeactivate) {
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
            Q_EMIT windowDeactivated();
        }
    }

    QQuickWindow::focusOutEvent(ev);
}

void Dialog::showEvent(QShowEvent *event)
{
    d->updateResizableEdges();
    d->updateSizeFromAppletInterface();

    if (d->backgroundHints != Dialog::NoBackground) {
        DialogShadows::instance()->addWindow(this, d->dialogBackground->enabledBorders());
    }

    if (KWindowSystem::isPlatformX11()) {
        KX11Extras::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    }
    QQuickWindow::showEvent(event);
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        if (!KWindowSystem::isPlatformWayland() || isRunningInKWin() || !isExposed()) {
            return QQuickWindow::event(event);
        }

        /*
         * expose event is the only place where to correctly
         * register our wayland extensions, as showevent is a bit too
         * soon and the platform window isn't shown yet
         * (only the first expose event, guarded by needsSetupNextExpose bool)
         * and tear it down when the window gets hidden
         * see https://phabricator.kde.org/T6064
         */
        // sometimes non null regions arrive even for non visible windows
        // for which surface creation would fail
        if (d->needsSetupNextExpose && isVisible()) {
            d->updateVisibility(true);
            const bool ret = QQuickWindow::event(event);
            d->updateTheme();
            d->needsSetupNextExpose = false;
            return ret;
        }
    } else if (event->type() == QEvent::Show) {
        d->updateVisibility(true);
    } else if (event->type() == QEvent::Hide) {
        d->updateVisibility(false);
        d->needsSetupNextExpose = true;
    } else if (event->type() == QEvent::Move) {
        QMoveEvent *me = static_cast<QMoveEvent *>(event);
        LingmoShellWaylandIntegration::get(this)->setPosition(me->pos());
    }

    /*Fitt's law: if the containment has margins, and the mouse cursor clicked
     * on the mouse edge, forward the click in the containment boundaries
     */
    if (d->mainItem && !d->mainItem->size().isEmpty()) {
        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (d->resizableEdges) {
                if (event->type() == QEvent::MouseMove && d->updateMouseCursor(me->globalPosition())) {
                    return QQuickWindow::event(event);
                }
                if (event->type() == QEvent::MouseButtonPress) {
                    const QPointF globalMousePos = me->globalPosition();
                    const Qt::Edges sides = d->hitTest(globalMousePos) & d->resizableEdges;
                    if (sides) {
                        startSystemResize(sides);
                        return true;
                    }
                }
            }

            // don't mess with position if the cursor is actually outside the view:
            // somebody is doing a click and drag that must not break when the cursor is outside
            if (geometry().contains(me->globalPosition().toPoint()) && !d->mainItemContainsPosition(me->scenePosition())) {
                QMouseEvent me2(me->type(),
                                d->positionAdjustedForMainItem(me->scenePosition()),
                                d->positionAdjustedForMainItem(me->scenePosition()),
                                d->positionAdjustedForMainItem(me->scenePosition()) + position(),
                                me->button(),
                                me->buttons(),
                                me->modifiers());
                me2.setTimestamp(me->timestamp());

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
                we2.setTimestamp(we->timestamp());

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &we2);
                }
                return true;
            }
            break;
        }

        case QEvent::DragEnter: {
            QDragEnterEvent *de = static_cast<QDragEnterEvent *>(event);
            if (!d->mainItemContainsPosition(de->position())) {
                QDragEnterEvent de2(d->positionAdjustedForMainItem(de->position()).toPoint(),
                                    de->possibleActions(),
                                    de->mimeData(),
                                    de->buttons(),
                                    de->modifiers());

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
            if (!d->mainItemContainsPosition(de->position())) {
                QDragMoveEvent de2(d->positionAdjustedForMainItem(de->position()).toPoint(),
                                   de->possibleActions(),
                                   de->mimeData(),
                                   de->buttons(),
                                   de->modifiers());

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &de2);
                }
                return true;
            }
            break;
        }
        case QEvent::Drop: {
            QDropEvent *de = static_cast<QDropEvent *>(event);
            if (!d->mainItemContainsPosition(de->position())) {
                QDropEvent de2(d->positionAdjustedForMainItem(de->position()).toPoint(), de->possibleActions(), de->mimeData(), de->buttons(), de->modifiers());

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

void Dialog::hideEvent(QHideEvent *event)
{
    // Persist the size if this contains an applet
    if (d->appletInterface && d->mainItem) {
        KConfigGroup config = d->appletInterface->applet()->config();
        qreal w = d->mainItem->width();
        qreal h = d->mainItem->height();
        config.writeEntry("popupWidth", w);
        config.writeEntry("popupHeight", h);
        config.sync();
    }

    QQuickWindow::hideEvent(event);
}

void Dialog::moveEvent(QMoveEvent *e)
{
    QQuickWindow::moveEvent(e);
    if (d->resizableEdges) {
        d->updateMouseCursor(QCursor::pos());
    }
}

void Dialog::classBegin()
{
    d->componentComplete = false;
}

void Dialog::componentComplete()
{
    d->componentComplete = true;
    QQuickWindow::setVisible(d->visible);
    d->updateTheme();
}

bool Dialog::hideOnWindowDeactivate() const
{
    return d->hideOnWindowDeactivate;
}

void Dialog::setHideOnWindowDeactivate(bool hide)
{
    if (d->hideOnWindowDeactivate == hide) {
        return;
    }
    d->hideOnWindowDeactivate = hide;
    Q_EMIT hideOnWindowDeactivateChanged();
}

bool Dialog::isOutputOnly() const
{
    return d->outputOnly;
}

void Dialog::setOutputOnly(bool outputOnly)
{
    if (d->outputOnly == outputOnly) {
        return;
    }
    d->outputOnly = outputOnly;
    d->applyType();
    Q_EMIT outputOnlyChanged();
}

void Dialog::setFloating(int floating)
{
    d->floating = floating;
    Q_EMIT floatingChanged();
}

int Dialog::floating() const
{
    return d->floating;
}

void Dialog::setVisible(bool visible)
{
    // only update real visibility when we have finished component completion
    // and all flags have been set

    d->visible = visible;
    if (d->componentComplete) {
        if (visible && d->visualParent) {
            setPosition(popupPosition(d->visualParent, size()));
        }

        // Bug 381242: Qt remembers minimize state and re-applies it when showing
        setWindowStates(windowStates() & ~Qt::WindowMinimized);
        QQuickWindow::setVisible(visible);
        // signal will be emitted and proxied from the QQuickWindow code
    } else {
        Q_EMIT visibleChangedProxy();
    }
}

bool Dialog::isVisible() const
{
    if (d->componentComplete) {
        return QQuickWindow::isVisible();
    }
    return d->visible;
}

void Dialog::setAppletInterface(QQuickItem *appletInterface)
{
    if (d->appletInterface == appletInterface) {
        return;
    }
    d->appletInterface = qobject_cast<AppletQuickItem *>(appletInterface);
    Q_EMIT appletInterfaceChanged();
}

QQuickItem *Dialog::appletInterface() const
{
    return d->appletInterface;
}

Dialog::BackgroundHints Dialog::backgroundHints() const
{
    return d->backgroundHints;
}

void Dialog::setBackgroundHints(Dialog::BackgroundHints hints)
{
    if (d->backgroundHints == hints) {
        return;
    }

    d->backgroundHints = hints;
    d->updateTheme();
    Q_EMIT backgroundHintsChanged();
}

}

#include "moc_dialog.cpp"
