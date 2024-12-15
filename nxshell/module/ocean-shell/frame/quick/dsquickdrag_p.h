// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QQmlComponent>
#include <QObject>
#include <DObject>

class QWindow;
DS_BEGIN_NAMESPACE

class DQuickDragPrivate;
class DQuickDrag : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(QQmlComponent* overlay READ overlay WRITE setOverlay NOTIFY overlayChanged FINAL)
    Q_PROPERTY(QSizeF hotSpotScale READ hotSpotScale WRITE setHotSpotScale RESET resetHotSpotScale NOTIFY hotSpotScaleChanged FINAL)
    Q_PROPERTY(QPoint startDragPoint READ startDragPoint NOTIFY startDragPointChanged FINAL)
    Q_PROPERTY(QPoint currentDragPoint READ currentDragPoint NOTIFY currentDragPointChanged FINAL)
    Q_PROPERTY(QWindow* overlayWindow READ overlayWindow NOTIFY overlayWindowChanged FINAL)
    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged FINAL)
    D_DECLARE_PRIVATE(DQuickDrag)
public:
    explicit DQuickDrag(QObject *parent = nullptr);

    bool active() const;
    void setActive(bool newActive);

    QQmlComponent *overlay() const;
    void setOverlay(QQmlComponent *newOverlay);

    QSizeF hotSpotScale() const;
    void setHotSpotScale(const QSizeF &newHotSpot);
    void resetHotSpotScale();

    QPoint startDragPoint() const;
    QPoint currentDragPoint() const;

    QWindow* overlayWindow() const;
    bool isDragging() const;

    static DQuickDrag *qmlAttachedProperties(QObject *object);
signals:
    void activeChanged();
    void startDragPointChanged();
    void overlayChanged();
    void hotSpotScaleChanged();
    void currentDragPointChanged();
    void overlayWindowChanged();
    void isDraggingChanged();
};

DS_END_NAMESPACE

QML_DECLARE_TYPEINFO(DS_NAMESPACE::DQuickDrag, QML_HAS_ATTACHED_PROPERTIES)
