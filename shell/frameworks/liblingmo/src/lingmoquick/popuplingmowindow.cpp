/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "popuplingmowindow.h"

#include <kwindoweffects.h>
#include <kwindowsystem.h>

#include "debug_p.h"
#include <QGuiApplication>
#include <QScreen>
#include <qnamespace.h>
#include <qtmetamacros.h>

#include "lingmoshellwaylandintegration.h"
#include "transientplacementhint_p.h"
#include "utils.h"

namespace LingmoQuick
{

class PopupLingmoWindowPrivate
{
public:
    PopupLingmoWindowPrivate(PopupLingmoWindow *_q);

    void updateEffectivePopupDirection(const QRect &anchorRect, const QRect &relativePopupPosition);
    void updateSlideEffect();
    void updatePosition();
    void updatePositionX11(const QPoint &position);
    void updatePositionWayland(const QPoint &position);
    void updateBorders(const QRect &globalPosition);
    void updateVisualParentWindow();

    PopupLingmoWindow *q;
    QPointer<QQuickItem> m_visualParent;
    QPointer<QQuickWindow> m_visualParentWindow;
    PopupLingmoWindow::RemoveBorders m_removeBorderStrategy = PopupLingmoWindow::Never;
    bool m_needsReposition = false;
    bool m_floating = false;
    bool m_animated = false;
    int m_margin = 0;
    Qt::Edge m_popupDirection = Qt::TopEdge;
    Qt::Edge m_effectivePopupDirection = Qt::TopEdge;
};

PopupLingmoWindowPrivate::PopupLingmoWindowPrivate(PopupLingmoWindow *_q)
    : q(_q)
{
}

/**
 * PopupLingmoWindowPrivate::updateSlideEffect
 * @param anchorRect - the rect around where the popup should be placed relative to the parent window
 * @param relativePopupPosition - the final rect of the popup relative to the parent window
 *
 * This is based purely on position in prepartion for being called in a wayland configure event
 */
void PopupLingmoWindowPrivate::updateEffectivePopupDirection(const QRect &anchorRect, const QRect &relativePopupPosition)
{
    Qt::Edge effectivePopupDirection = Qt::TopEdge;
    if (m_popupDirection == Qt::TopEdge || m_popupDirection == Qt::BottomEdge) {
        if (relativePopupPosition.center().y() >= anchorRect.center().y()) {
            effectivePopupDirection = Qt::BottomEdge;
        } else {
            effectivePopupDirection = Qt::TopEdge;
        }
    }
    if (m_popupDirection == Qt::LeftEdge || m_popupDirection == Qt::RightEdge) {
        if (relativePopupPosition.center().x() >= anchorRect.center().x()) {
            effectivePopupDirection = Qt::RightEdge;
        } else {
            effectivePopupDirection = Qt::LeftEdge;
        }
    }

    if (effectivePopupDirection != m_effectivePopupDirection) {
        Q_EMIT q->effectivePopupDirectionChanged();
        m_effectivePopupDirection = effectivePopupDirection;
    }
}

void PopupLingmoWindowPrivate::updateSlideEffect()
{
    KWindowEffects::SlideFromLocation slideLocation = KWindowEffects::NoEdge;
    if (!m_animated) {
        KWindowEffects::slideWindow(q, slideLocation);
        return;
    }
    switch (m_effectivePopupDirection) {
    case Qt::TopEdge:
        slideLocation = KWindowEffects::BottomEdge;
        break;
    case Qt::BottomEdge:
        slideLocation = KWindowEffects::TopEdge;
        break;
    case Qt::LeftEdge:
        slideLocation = KWindowEffects::RightEdge;
        break;
    case Qt::RightEdge:
        slideLocation = KWindowEffects::LeftEdge;
        break;
    }
    KWindowEffects::slideWindow(q, slideLocation);
}

void PopupLingmoWindowPrivate::updatePosition()
{
    m_needsReposition = false;

    if (!m_visualParent || !m_visualParent->window()) {
        qCWarning(LOG_LINGMOQUICK) << "Exposed with no visual parent. Window positioning broken.";
        return;
    }
    q->setTransientParent(m_visualParent->window());
    TransientPlacementHint placementHint;
    QRectF parentAnchorRect = QRectF(m_visualParent->mapToScene(QPointF(0, 0)), m_visualParent->size());

    if (!m_floating) {
        QRect windowVisibleRect = m_visualParent->window()->mask().boundingRect();
        // pad parentAnchorRect to the window it's in, so that the popup appears outside the panel
        // even if the tooltip area does not fill it
        if (m_popupDirection == Qt::TopEdge || m_popupDirection == Qt::BottomEdge) {
            parentAnchorRect.setTop(windowVisibleRect.top());
            // QRect::bottom() is off by one
            parentAnchorRect.setBottom(windowVisibleRect.bottom() + 1);
        }
        if (m_popupDirection == Qt::LeftEdge || m_popupDirection == Qt::RightEdge) {
            parentAnchorRect.setLeft(windowVisibleRect.left());
            // QRect::right() is off by one
            parentAnchorRect.setRight(windowVisibleRect.right() + 1);
        }
    }

    placementHint.setParentAnchorArea(parentAnchorRect.toRect());
    placementHint.setParentAnchor(m_popupDirection);
    placementHint.setPopupAnchor(LingmoQuickPrivate::oppositeEdge(m_popupDirection));
    placementHint.setFlipConstraintAdjustments(m_floating ? Qt::Vertical : Qt::Orientations());
    placementHint.setMargin(m_margin);

    const QRect popupPosition = TransientPlacementHelper::popupRect(q, placementHint);

    QRect relativePopupPosition = popupPosition;
    if (m_visualParent->window()) {
        relativePopupPosition = relativePopupPosition.translated(-m_visualParent->window()->position());
    }
    updateEffectivePopupDirection(parentAnchorRect.toRect(), relativePopupPosition);
    updateSlideEffect();

    if (KWindowSystem::isPlatformX11()) {
        updatePositionX11(popupPosition.topLeft());
    } else if (KWindowSystem::isPlatformWayland()) {
        updatePositionWayland(popupPosition.topLeft());
    }

    updateBorders(popupPosition);
}

void PopupLingmoWindowPrivate::updatePositionX11(const QPoint &position)
{
    q->setPosition(position);
}

void PopupLingmoWindowPrivate::updatePositionWayland(const QPoint &position)
{
    // still update's Qt internal reference as it's used by the next dialog
    // this can be dropped when we're using true semantic positioning in the backend
    q->setPosition(position);

    LingmoShellWaylandIntegration::get(q)->setPosition(position);
}

void PopupLingmoWindowPrivate::updateBorders(const QRect &globalPosition)
{
    // disables borders for the edges that are touching the screen edge

    QScreen *screen = QGuiApplication::screenAt(globalPosition.center());
    if (!screen) {
        return;
    }
    const QRect screenGeometry = screen->geometry();

    Qt::Edges enabledBorders = Qt::LeftEdge | Qt::RightEdge | Qt::TopEdge | Qt::BottomEdge;

    if (m_margin) {
        q->setBorders(enabledBorders);
        return;
    }

    if (m_removeBorderStrategy & PopupLingmoWindow::AtScreenEdges) {
        if (globalPosition.top() <= screenGeometry.top()) {
            enabledBorders.setFlag(Qt::TopEdge, false);
        }
        if (globalPosition.bottom() >= screenGeometry.bottom()) {
            enabledBorders.setFlag(Qt::BottomEdge, false);
        }
        if (globalPosition.left() <= screenGeometry.left()) {
            enabledBorders.setFlag(Qt::LeftEdge, false);
        }
        if (globalPosition.right() >= screenGeometry.right()) {
            enabledBorders.setFlag(Qt::RightEdge, false);
        }
    }
    if (m_removeBorderStrategy & PopupLingmoWindow::AtPanelEdges) {
        switch (m_popupDirection) {
        case Qt::LeftEdge:
            enabledBorders.setFlag(Qt::RightEdge, false);
            break;
        case Qt::RightEdge:
            enabledBorders.setFlag(Qt::LeftEdge, false);
            break;
        case Qt::BottomEdge:
            enabledBorders.setFlag(Qt::TopEdge, false);
            break;
        case Qt::TopEdge:
        default:
            enabledBorders.setFlag(Qt::BottomEdge, false);
            break;
        }
    }
    q->setBorders(enabledBorders);
}

void PopupLingmoWindowPrivate::updateVisualParentWindow()
{
    if (m_visualParentWindow) {
        QObject::disconnect(m_visualParentWindow, &QQuickWindow::yChanged, q, &PopupLingmoWindow::queuePositionUpdate);
        QObject::disconnect(m_visualParentWindow, &QQuickWindow::xChanged, q, &PopupLingmoWindow::queuePositionUpdate);
    }

    m_visualParentWindow = m_visualParent ? m_visualParent->window() : nullptr;

    if (m_visualParentWindow) {
        QObject::connect(m_visualParentWindow, &QQuickWindow::yChanged, q, &PopupLingmoWindow::queuePositionUpdate);
        QObject::connect(m_visualParentWindow, &QQuickWindow::xChanged, q, &PopupLingmoWindow::queuePositionUpdate);
    }
}

PopupLingmoWindow::PopupLingmoWindow(const QString &svgPrefix)
    : LingmoWindow(svgPrefix)
    , d(new PopupLingmoWindowPrivate(this))
{
}

PopupLingmoWindow::~PopupLingmoWindow()
{
}

void PopupLingmoWindow::setVisualParent(QQuickItem *item)
{
    if (item == d->m_visualParent) {
        return;
    }

    if (d->m_visualParent) {
        disconnect(d->m_visualParent, SIGNAL(windowChanged(QQuickWindow *)), this, SLOT(updateVisualParentWindow()));
    }

    d->m_visualParent = item;
    d->updateVisualParentWindow();

    if (d->m_visualParent) {
        connect(d->m_visualParent, SIGNAL(windowChanged(QQuickWindow *)), this, SLOT(updateVisualParentWindow()));
    }

    Q_EMIT visualParentChanged();
    queuePositionUpdate();
}

QQuickItem *PopupLingmoWindow::visualParent() const
{
    return d->m_visualParent;
}

Qt::Edge PopupLingmoWindow::popupDirection() const
{
    return d->m_popupDirection;
}

void PopupLingmoWindow::setPopupDirection(Qt::Edge popupDirection)
{
    if (popupDirection == d->m_popupDirection) {
        return;
    }
    d->m_popupDirection = popupDirection;

    if (isExposed()) {
        qCWarning(LOG_LINGMOQUICK) << "location should be set before showing popup window";
    }
    queuePositionUpdate();

    Q_EMIT popupDirectionChanged();
}

Qt::Edge PopupLingmoWindow::effectivePopupDirection() const
{
    return d->m_effectivePopupDirection;
}

bool PopupLingmoWindow::floating() const
{
    return d->m_floating;
}

void PopupLingmoWindow::setFloating(bool floating)
{
    if (floating == d->m_floating) {
        return;
    }
    d->m_floating = floating;
    queuePositionUpdate();
    Q_EMIT floatingChanged();
}

bool PopupLingmoWindow::animated() const
{
    return d->m_animated;
}

void PopupLingmoWindow::setAnimated(bool animated)
{
    d->m_animated = animated;
    queuePositionUpdate();
    Q_EMIT animatedChanged();
}

PopupLingmoWindow::RemoveBorders PopupLingmoWindow::removeBorderStrategy() const
{
    return d->m_removeBorderStrategy;
}

void PopupLingmoWindow::setRemoveBorderStrategy(PopupLingmoWindow::RemoveBorders strategy)
{
    if (d->m_removeBorderStrategy == strategy) {
        return;
    }

    d->m_removeBorderStrategy = strategy;
    queuePositionUpdate(); // This will update borders as well
    Q_EMIT removeBorderStrategyChanged();
}

int PopupLingmoWindow::margin() const
{
    return d->m_margin;
}

void PopupLingmoWindow::setMargin(int margin)
{
    if (d->m_margin == margin) {
        return;
    }

    d->m_margin = margin;
    queuePositionUpdate();
    Q_EMIT marginChanged();
}

bool PopupLingmoWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        if (d->m_needsReposition) {
            d->updatePosition();
        }
        break;
    case QEvent::Show:
        d->updatePosition();
        break;
    case QEvent::Resize:
        d->updatePosition();
        break;
    default:
        break;
    }
    return LingmoQuick::LingmoWindow::event(event);
}

void PopupLingmoWindow::queuePositionUpdate()
{
    d->m_needsReposition = true;
}
}

#include "moc_popuplingmowindow.cpp"
