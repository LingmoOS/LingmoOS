/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
 */

#ifndef ICONLABELLAYOUT_P_H
#define ICONLABELLAYOUT_P_H

#include "iconlabellayout.h"

class IconLabelLayoutPrivate : public QObject
{
    Q_DECLARE_PUBLIC(IconLabelLayout)
    Q_DISABLE_COPY(IconLabelLayoutPrivate)

public:
    IconLabelLayoutPrivate(IconLabelLayout *qq)
        : q_ptr(qq)
    {
    }

    bool createIconItem();
    bool destroyIconItem();
    bool updateIconItem();
    void syncIconItem();
    void updateOrSyncIconItem();

    bool createLabelItem();
    bool destroyLabelItem();
    bool updateLabelItem();
    void syncLabelItem();
    void updateOrSyncLabelItem();

    void updateImplicitSize();
    void layout();

    IconLabelLayout *const q_ptr;

    QPointer<QQmlComponent> iconComponent;
    QPointer<QQmlComponent> labelComponent;

    QPointer<QQuickItem> iconItem;
    QPointer<QQuickItem> labelItem;

    bool hasIcon = false;
    bool hasLabel = false;

    Ocean::QQuickIcon icon = Ocean::QQuickIcon();
    QString text = QString();
    QFont font = QFont();
    QColor color = QColor();

    qreal availableWidth = 0.0;
    qreal availableHeight = 0.0;

    qreal spacing = 0.0;
    qreal leftPadding = 0.0;
    qreal rightPadding = 0.0;
    qreal topPadding = 0.0;
    qreal bottomPadding = 0.0;

    bool mirrored = false;
    Qt::Alignment alignment = Qt::AlignCenter;
    IconLabelLayout::Display display = IconLabelLayout::TextBesideIcon;

    QRectF iconRect = QRectF(0, 0, 0, 0);
    QRectF labelRect = QRectF(0, 0, 0, 0);
    qreal contentWidth = 0.0;
    qreal contentHeight = 0.0;

    bool firstLayoutCompleted = false;
    int layoutCount = 0;
};

#endif
