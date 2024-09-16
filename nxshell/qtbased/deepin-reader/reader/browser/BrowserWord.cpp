// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserWord.h"
#include "BrowserPage.h"
#include "Model.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

BrowserWord::BrowserWord(QGraphicsItem *parent, deepin_reader::Word word) : QGraphicsItem(parent), m_word(word)
{
    setZValue(deepin_reader::Z_ORDER_SELECT_TEXT);
}

void BrowserWord::setScaleFactor(qreal scaleFactor)
{
    m_scaleFactor = scaleFactor;
    update();
}

QString BrowserWord::text()
{
    return m_word.text;
}

void BrowserWord::setSelectable(bool enable)
{
    m_selectable = enable;

    setFlag(QGraphicsItem::ItemIsSelectable, enable);
}

QRectF BrowserWord::boundingRect() const
{
    return QRectF(m_word.boundingBox.x() * m_scaleFactor - 1, m_word.boundingBox.y() * m_scaleFactor - 1, m_word.boundingBox.width() * m_scaleFactor + 2, m_word.boundingBox.height() * m_scaleFactor + 2);
}

QRectF BrowserWord::boundingBox() const
{
    return m_word.boundingBox;
}

void BrowserWord::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (isSelected()) {
        painter->setBrush(QColor(72, 118, 255, 100));
        painter->setPen(Qt::NoPen);
        painter->drawRect(option->rect.x() + 1, option->rect.y() + 1, option->rect.width() - 2, option->rect.height() - 2);
    }
}

void BrowserWord::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if (m_selectable)
        return;     //在可以选中的情况下不允许单选

    QGraphicsItem::mousePressEvent(e);
}

void BrowserWord::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    if (m_selectable)
        return;     //在可以选中的情况下不允许单选

    QGraphicsItem::mouseReleaseEvent(e);
}
