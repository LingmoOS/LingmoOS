// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserAnnotation.h"
#include "BrowserPage.h"

#include <DApplicationHelper>

#include <QPainter>
#include <QStyleOptionGraphicsItem>

BrowserAnnotation::BrowserAnnotation(QGraphicsItem *parent, QRectF rect, deepin_reader::Annotation *annotation, qreal scalefactor) : QGraphicsItem(parent),
    m_annotation(annotation), m_rect(rect), m_parent(parent), m_scaleFactor(scalefactor)
{
    if (nullptr == m_annotation)
        return;

    if (m_annotation->type() == deepin_reader::Annotation::AText)
        setZValue(deepin_reader::Z_ORDER_ICON);
    else
        setZValue(deepin_reader::Z_ORDER_HIGHLIGHT);
}

BrowserAnnotation::~BrowserAnnotation()
{
    m_annotation = nullptr;
}

int BrowserAnnotation::annotationType()
{
    if (nullptr == m_annotation)
        return -1;

    return m_annotation->type();
}

QString BrowserAnnotation::annotationText()
{
    if (nullptr == m_annotation)
        return QString();

    return m_annotation->contents();
}

QRectF BrowserAnnotation::boundingRect() const
{
    return QRectF(m_rect.x() * m_scaleFactor,
                  m_rect.y() * m_scaleFactor,
                  m_rect.width() * m_scaleFactor,
                  m_rect.height() * m_scaleFactor);
}

void BrowserAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (m_drawSelectRect) {
        QPen pen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        painter->setPen(pen);
        painter->drawRect(option->rect);
    }
}

deepin_reader::Annotation *BrowserAnnotation::annotation()
{
    return m_annotation;
}

bool BrowserAnnotation::isSame(Annotation *annotation)
{
    return (annotation == m_annotation);
}

void BrowserAnnotation::setDrawSelectRect(const bool draw)
{
    if (nullptr != m_annotation && m_annotation->type() != 1) {
        m_drawSelectRect = false;
        update();
        return;
    }

    m_drawSelectRect = draw;

    update();
}

void BrowserAnnotation::setScaleFactor(const double scale)
{
    if (nullptr == m_annotation)
        return;

    m_scaleFactor = scale;

    update();
}
