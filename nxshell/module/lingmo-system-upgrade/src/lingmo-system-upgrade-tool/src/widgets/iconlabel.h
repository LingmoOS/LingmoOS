// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DWidget>

#include <QLabel>
#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QBoxLayout>
#include <QSpacerItem>

#include "../core/constants.h"

DWIDGET_USE_NAMESPACE

class IconLabel : public QWidget
{
public:
    QBoxLayout *m_layout;

    IconLabel(QWidget *parent, Orientation orientation = Orientation::Horizontal, Qt::Alignment alignment = Qt::AlignCenter);
    void addWidget(QWidget *widget, Qt::Alignment alignment = Qt::AlignCenter);
    void addLayout(QLayout *layout);
    void addSpacing(int sz);
    void addStretch(int sz);
    void addSpacerItem(QSpacerItem *item);
    void setIcon(QString qstring);
    // Method for SVG Icons
    void setIcon(QString qstring, int w, int h);
    void setIcon(QPixmap pixmap);
    void setIconSize(const QSize &size);
    void scaleToWidth(int width);
    void scaleToHeight(int height);
    void scale(
        int width,
        int height,
        Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio,
        Qt::TransformationMode transformMode = Qt::FastTransformation
    );


protected:
    QLabel          *m_label;
    Qt::Alignment   m_defaultAlignment;

    void initUI();
};
