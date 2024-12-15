// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cweekdaybackgrounditem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QMarginsF>

CWeekDayBackgroundItem::CWeekDayBackgroundItem(QGraphicsItem *parent)
    : CSceneBackgroundItem(CSceneBackgroundItem::OnWeekView, parent)
    , m_drawDividingLine(false)
    , m_showFocus(false)
{
}

void CWeekDayBackgroundItem::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_weekColor = "#00429A";
        m_weekColor.setAlphaF(0.05);
    } else {
        m_weekColor = "#4F9BFF";
        m_weekColor.setAlphaF(0.1);
    }
}

void CWeekDayBackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing);
    if (m_drawDividingLine) {
        //绘制分割线
        if (this->getBackgroundNum() != 6) {
            // 分割线颜色
            QColor m_linecolor = "#000000";
            m_linecolor.setAlphaF(0.05);
            painter->setPen(Qt::SolidLine);
            painter->setPen(m_linecolor);
            painter->drawLine(this->rect().topRight(), this->rect().bottomRight());
        }
        //绘制周六周日背景
        if (m_Date.dayOfWeek() > 5) {
            painter->setBrush(m_weekColor);
            painter->setPen(Qt::NoPen);
            painter->drawRect(this->rect());
        }
    }
    if (m_showFocus && getItemFocus()) {
        QPen framePen;
        //设置边框宽度
        framePen.setWidth(2);
        //设置边框颜色
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
        painter->setBrush(Qt::NoBrush);
        QRectF drawrect = this->rect().marginsRemoved(QMarginsF(1, 1, 1, 1));
        painter->drawRect(drawrect);
    }
}

void CWeekDayBackgroundItem::updateCurrentItemShow()
{
    if (m_showItemIndex >= 0) {
        if (m_item.size() > 0) {
            m_showItemIndex = m_showItemIndex < m_item.size() ? m_showItemIndex : 0;
            m_item.at(m_showItemIndex)->setItemFocus(true);
            //定位到当前焦点item
            emit signalPosOnView(m_item.at(m_showItemIndex)->rect().y());
        } else {
            m_showItemIndex = -1;
            setItemFocus(true);
        }
    }
}

bool CWeekDayBackgroundItem::showFocus() const
{
    return m_showFocus;
}

void CWeekDayBackgroundItem::setShowFocus(bool showFocus)
{
    m_showFocus = showFocus;
}

void CWeekDayBackgroundItem::setItemFocus(bool isFocus)
{
    //如果改背景不接受焦点切第一次设置进入该背景则设置该背景上第一个item接收focus
    if (m_showFocus == false && m_showItemIndex < 0) {
        if (hasNextSubItem()) {
            ++m_showItemIndex;
            m_item.at(m_showItemIndex)->setItemFocus(isFocus);
        }
    } else {
        CSceneBackgroundItem::setItemFocus(isFocus);
    }
}

bool CWeekDayBackgroundItem::hasNextSubItem()
{
    bool result = true;
    if (m_showItemIndex < 0 && getShowItemCount() == 0) {
        return false;
    }
    if (m_showItemIndex == getShowItemCount() - 1) {
        return false;
    }
    return result;
}

bool CWeekDayBackgroundItem::drawDividingLine() const
{
    return m_drawDividingLine;
}

void CWeekDayBackgroundItem::setDrawDividingLine(bool drawDividingLine)
{
    m_drawDividingLine = drawDividingLine;
}
