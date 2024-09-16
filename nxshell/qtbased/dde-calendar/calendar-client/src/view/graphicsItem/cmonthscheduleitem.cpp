// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cmonthscheduleitem.h"

#include <QPainter>

CMonthScheduleItem::CMonthScheduleItem(QRect rect, QGraphicsItem *parent, int edittype)
    : DragInfoItem(rect, parent)
    , m_pos(13, 5)
{
    Q_UNUSED(edittype);
}

CMonthScheduleItem::~CMonthScheduleItem()
{
}

QPixmap CMonthScheduleItem::getPixmap()
{
    QPixmap pixmap(this->rect().size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    paintBackground(&painter, pixmap.rect(), true);
    return pixmap;
}

void CMonthScheduleItem::paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap)
{
    qreal labelwidth = rect.width();
    qreal labelheight = rect.height();
    m_font = DFontSizeManager::instance()->get(m_sizeType, m_font);
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    //根据类型获取颜色
    CSchedulesColor gdColor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo->scheduleTypeID());
    QColor brushColor = gdColor.normalColor;
    QColor textcolor = CScheduleDataManage::getScheduleDataManage()->getTextColor();

    //判断是否为选中日程
    if (!m_vScheduleInfo.isNull() && m_vScheduleInfo == m_pressInfo) {
        //判断当前日程是否为拖拽移动日程
        if (m_vScheduleInfo->isMoved() == m_pressInfo->isMoved()) {
            m_vHighflag = true;
        } else {
            painter->setOpacity(0.4);
            textcolor.setAlphaF(0.4);
        }
        m_vSelectflag = m_press;
    }

    if (isPixMap) {
        painter->setOpacity(0.6);
        textcolor.setAlphaF(0.8);
    }

    if (m_vSelectflag) {
        brushColor = gdColor.pressColor;
        textcolor.setAlphaF(0.4);
    } else if (m_vHoverflag) {
        brushColor = gdColor.hoverColor;
    } else if (m_vHighflag) {
        brushColor = gdColor.pressColor;
    }

    QRectF fillRect = QRectF(rect.x() + 2,
                             rect.y() + 2,
                             labelwidth - 2,
                             labelheight - 2);
    painter->save();
    //将直线开始点设为0，终点设为1，然后分段设置颜色
    painter->setBrush(brushColor);
    if (getItemFocus() && isPixMap == false) {
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(fillRect,
                             rect.height() / 3,
                             rect.height() / 3);
    painter->restore();
    painter->setFont(m_font);
    painter->setPen(textcolor);
    QFontMetrics fm = painter->fontMetrics();

    QString tSTitleName = m_vScheduleInfo->summary();
    tSTitleName.replace("\n", "");
    QString str = tSTitleName;
    //右侧偏移8
    qreal textWidth = labelwidth - m_pos.x() - m_offset * 2 - 8;
    QString tStr;
    int _rightOffset = fm.width("...");
    //显示宽度  左侧偏移13右侧偏移8
    qreal _showWidth = textWidth;
    //如果标题总长度大于显示长度则显示长度须减去"..."的长度
    if (fm.width(str) > _showWidth) {
        _showWidth -= _rightOffset;
        for (int i = 0; i < str.count(); i++) {
            tStr.append(str.at(i));
            int widthT = fm.width(tStr);
            //如果宽度大于显示长度则去除最后添加的字符
            if (widthT > _showWidth) {
                tStr.chop(1);
                break;
            }
        }
        if (tStr != str) {
            tStr = tStr + "...";
        }
    } else {
        tStr = str;
    }

    painter->drawText(QRectF(rect.x() + m_pos.x(),
                             rect.y() + 1,
                             textWidth,
                             labelheight - m_pos.y() + 3),
                      Qt::AlignLeft | Qt::AlignVCenter, tStr);

    if (m_vHoverflag && !m_vSelectflag) {
        QRectF tRect = QRectF(rect.x() + 2.5, rect.y() + 2.5, labelwidth - 3, labelheight - 3);
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing);
        QPen pen;
        QColor selcolor;

        if (themetype == 2) {
            selcolor = "#FFFFFF";
        } else {
            selcolor = "#000000";
        }

        selcolor.setAlphaF(0.08);

        pen.setColor(selcolor);
        pen.setWidthF(1);
        pen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRoundedRect(tRect, rect.height() / 3, rect.height() / 3);
        painter->restore();
    }

    if (m_vSelectflag) {
        QColor selcolor = "#000000";
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, rect.height() / 3, rect.height() / 3);
    }
}
