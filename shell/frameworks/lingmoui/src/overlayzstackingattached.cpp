/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "overlayzstackingattached.h"

#include "loggingcategory.h"

#include <QQuickItem>

OverlayZStackingAttached::OverlayZStackingAttached(QObject *parent)
    : QObject(parent)
    , m_layer(defaultLayerForPopupType(parent))
    , m_parentPopup(nullptr)
    , m_pending(false)
{
    Q_ASSERT(parent);
    if (!isPopup(parent)) {
        qCWarning(LingmoUILog) << "OverlayZStacking must be attached to a Popup";
        return;
    }

    connect(parent, SIGNAL(parentChanged()), this, SLOT(updateParentPopup()));
    connect(parent, SIGNAL(closed()), this, SLOT(dispatchPendingSignal()));
    // Note: aboutToShow is too late, as QQuickPopup has already created modal
    // dimmer based off current z index.
}

OverlayZStackingAttached::~OverlayZStackingAttached()
{
}

qreal OverlayZStackingAttached::z() const
{
    if (!m_parentPopup) {
        const_cast<OverlayZStackingAttached *>(this)->updateParentPopupSilent();
    }

    qreal layerZ = defaultZForLayer(m_layer);
    qreal parentZ = parentPopupZ() + 1;

    return std::max(layerZ, parentZ);
}

OverlayZStackingAttached::Layer OverlayZStackingAttached::layer() const
{
    return m_layer;
}

void OverlayZStackingAttached::setLayer(Layer layer)
{
    if (m_layer == layer) {
        return;
    }

    m_layer = layer;
    Q_EMIT layerChanged();
    enqueueSignal();
}

OverlayZStackingAttached *OverlayZStackingAttached::qmlAttachedProperties(QObject *object)
{
    return new OverlayZStackingAttached(object);
}

void OverlayZStackingAttached::enqueueSignal()
{
    if (isVisible(parent())) {
        m_pending = true;
    } else {
        Q_EMIT zChanged();
    }
}

void OverlayZStackingAttached::dispatchPendingSignal()
{
    if (m_pending) {
        m_pending = false;
        Q_EMIT zChanged();
    }
}

void OverlayZStackingAttached::updateParentPopup()
{
    const qreal oldZ = parentPopupZ();

    updateParentPopupSilent();

    if (oldZ != parentPopupZ()) {
        enqueueSignal();
    }
}

void OverlayZStackingAttached::updateParentPopupSilent()
{
    auto popup = findParentPopup(parent());
    setParentPopup(popup);
}

void OverlayZStackingAttached::setParentPopup(QObject *parentPopup)
{
    if (m_parentPopup == parentPopup) {
        return;
    }

    if (m_parentPopup) {
        disconnect(m_parentPopup.data(), SIGNAL(zChanged()), this, SLOT(enqueueSignal()));
    }

    // Ideally, we would also connect to all the parent items' parentChanged() on the way to parent popup.
    m_parentPopup = parentPopup;

    if (m_parentPopup) {
        connect(m_parentPopup.data(), SIGNAL(zChanged()), this, SLOT(enqueueSignal()));
    }
}

qreal OverlayZStackingAttached::parentPopupZ() const
{
    if (m_parentPopup) {
        return m_parentPopup->property("z").toReal();
    }
    return -1;
}

bool OverlayZStackingAttached::isVisible(const QObject *popup)
{
    if (!isPopup(popup)) {
        return false;
    }

    return popup->property("visible").toBool();
}

bool OverlayZStackingAttached::isPopup(const QObject *object)
{
    return object && object->inherits("QQuickPopup");
}

QObject *OverlayZStackingAttached::findParentPopup(const QObject *popup)
{
    auto item = findParentPopupItem(popup);
    if (!item) {
        return nullptr;
    }
    auto parentPopup = item->parent();
    if (!isPopup(popup)) {
        return nullptr;
    }
    return parentPopup;
}

QQuickItem *OverlayZStackingAttached::findParentPopupItem(const QObject *popup)
{
    if (!isPopup(popup)) {
        return nullptr;
    }

    QQuickItem *parentItem = popup->property("parent").value<QQuickItem *>();
    if (!parentItem) {
        return nullptr;
    }

    QQuickItem *item = parentItem;
    do {
        if (item && item->inherits("QQuickPopupItem")) {
            return item;
        }
        item = item->parentItem();
    } while (item);

    return nullptr;
}

OverlayZStackingAttached::Layer OverlayZStackingAttached::defaultLayerForPopupType(const QObject *popup)
{
    if (popup) {
        if (popup->inherits("QQuickDialog")) {
            return Layer::Dialog;
        } else if (popup->inherits("QQuickDrawer")) {
            return Layer::Drawer;
        } else if (popup->inherits("QQuickMenu")) {
            return Layer::Menu;
        } else if (popup->inherits("QQuickToolTip")) {
            return Layer::ToolTip;
        }
    }
    return DefaultLowest;
}

qreal OverlayZStackingAttached::defaultZForLayer(Layer layer)
{
    switch (layer) {
    case DefaultLowest:
        return 0;
    case Drawer:
        return 100;
    case FullScreen:
        return 200;
    case Dialog:
        return 300;
    case Menu:
        return 400;
    case Notification:
        return 500;
    case ToolTip:
        return 600;
    }
    return 0;
}

#include "moc_overlayzstackingattached.cpp"
