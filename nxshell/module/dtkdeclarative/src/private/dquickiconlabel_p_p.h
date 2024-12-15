// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKICONLABEL_P_P_H
#define DQUICKICONLABEL_P_P_H

#include "dquickiconlabel_p.h"

#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickIconLabelPrivate : public QQuickItemPrivate, public QQuickItemChangeListener
{
    Q_DECLARE_PUBLIC(DQuickIconLabel)

public:
    bool hasIcon() const;
    bool hasText() const;

    void createIconImage();
    bool ensureImage();
    bool destroyImage();
    bool updateImage();
    void syncImage();
    void updateOrSyncImage();

    bool createLabel();
    bool destroyLabel();
    bool updateLabel();
    void syncLabel();
    void updateOrSyncLabel();

    void updateImplicitSize();
    void layout();

    void watchChanges(QQuickItem *item);
    void unwatchChanges(QQuickItem *item);
    void setPositioningDirty();

    bool isLeftToRight() const;

    void itemImplicitWidthChanged(QQuickItem *) override;
    void itemImplicitHeightChanged(QQuickItem *) override;
    void itemDestroyed(QQuickItem *item) override;

    QSize iconSize() const;

    bool mirrored = false;
    DQuickIconLabel::Display display = DQuickIconLabel::TextBesideIcon;
    Qt::Alignment alignment = Qt::AlignCenter;
    qreal spacing = 0;
    qreal topPadding = 0;
    qreal leftPadding = 0;
    qreal rightPadding = 0;
    qreal bottomPadding = 0;
    QFont font;
    QColor color;
    QString text;
    DQuickDciIconImage *image = nullptr;
    QQuickText *label = nullptr;
    DQuickDciIcon icon;
};

DQUICK_END_NAMESPACE

#endif // DQUICKICONLABEL_P_P_H
