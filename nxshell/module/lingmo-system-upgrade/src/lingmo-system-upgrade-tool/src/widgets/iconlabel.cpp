// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>

#include "iconlabel.h"
#include "../tool/application.h"

IconLabel::IconLabel(QWidget *parent, Orientation orientation, Qt::Alignment alignment)
    : QWidget(parent)
    , m_defaultAlignment(alignment)
    , m_label(new QLabel(this))
    , m_layout(orientation == Orientation::Horizontal ? dynamic_cast<QBoxLayout*>(new QHBoxLayout(this)) : dynamic_cast<QBoxLayout*>(new QVBoxLayout(this)))
{
    initUI();
}

void IconLabel::initUI()
{
    m_label->setAlignment(m_defaultAlignment);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_label, Qt::AlignCenter);
    setLayout(m_layout);
    setContentsMargins(0, 0, 0, 0);
}

void IconLabel::setIcon(QString qstring)
{
    QPixmap pixmap(qstring);
    pixmap.setDevicePixelRatio(Application::getInstance()->devicePixelRatio());
    setIcon(pixmap);
}

void IconLabel::setIcon(QString qstring, int w, int h)
{
    setIcon(QIcon(qstring).pixmap(w, h));
}

void IconLabel::setIcon(QPixmap pixmap)
{
    m_label->setPixmap(pixmap);
}

void IconLabel::setIconSize(const QSize &size)
{
    m_label->setFixedSize(size);
}

void IconLabel::addWidget(QWidget *widget, Qt::Alignment alignment)
{
    m_layout->addWidget(widget, 0, alignment);
}

void IconLabel::addLayout(QLayout *layout)
{
    m_layout->addLayout(layout);
}

void IconLabel::addSpacing(int sz)
{
    m_layout->addSpacing(sz);
}

void IconLabel::addStretch(int sz)
{
    m_layout->addStretch(sz);
}

void IconLabel::addSpacerItem(QSpacerItem *item)
{
    m_layout->addSpacerItem(item);
}

void IconLabel::scaleToWidth(int width)
{
    setIcon(m_label->pixmap()->scaledToWidth(width));
}

void IconLabel::scaleToHeight(int height)
{
    setIcon(m_label->pixmap()->scaledToHeight(height));
}

void IconLabel::scale(
    int width,
    int height,
    Qt::AspectRatioMode aspectRatioMode,
    Qt::TransformationMode transformMode
) {
    setIcon(m_label->pixmap()->scaled(width, height, aspectRatioMode, transformMode));
}
