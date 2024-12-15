// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "modifyscheduleitem.h"

#include "../globaldef.h"

#include <QMouseEvent>

modifyScheduleItem::modifyScheduleItem(QWidget *parent)
    : ItemWidget(parent)
{
    setFixedHeight(MODIFY_ITEM_HEIGHT);
}

void modifyScheduleItem::DrawItem(QPainter &painter)
{
    drawIndex(painter);
    drawLine(painter);
    drawTitleContent(painter);
    drawScheduleColorLine(painter);
    drawTime(painter);
    drawDate(painter);
}

int modifyScheduleItem::getIndex() const
{
    return m_Index;
}

void modifyScheduleItem::setIndex(int Index)
{
    m_Index = Index;
}

void modifyScheduleItem::drawIndex(QPainter &painter)
{
    painter.save();
    painter.setFont(getTitleFont());
    painter.setPen(getDateTimeColor());

    const QRect indexRect(m_IndexX, 0, m_IndexWidth, this->rect().height());
    painter.drawText(indexRect, Qt::AlignCenter, QString::number(getIndex()));
    painter.restore();
}

void modifyScheduleItem::drawLine(QPainter &painter)
{
    painter.save();
    QRect rect(40, 10, 1, 28);
    painter.setPen(Qt::NoPen);
    painter.setBrush(LineColor());
    painter.drawRect(rect);
    painter.restore();
}

void modifyScheduleItem::drawTitleContent(QPainter &painter)
{
    painter.save();
    painter.setFont(getTitleFont());
    painter.setPen(getTitleColor());
    const int textRectWidth = this->rect().width() - m_TItleRightMargin - m_TitleX;
    const QRect titleContent(m_TitleX, 0, textRectWidth, this->rect().height());
    QFontMetrics metrics(getTitleFont());
    painter.drawText(titleContent,
                     Qt::AlignLeft | Qt::AlignVCenter,
                     metrics.elidedText(getTitleContent(), Qt::ElideRight, textRectWidth));
    painter.restore();
}

void modifyScheduleItem::drawScheduleColorLine(QPainter &painter)
{
    painter.save();
    QRect rect(this->width() - m_TItleRightMargin,
               0,
               2,
               this->height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(ScheduleColor());
    painter.drawRect(rect);
    painter.restore();
}

void modifyScheduleItem::drawTime(QPainter &painter)
{
    painter.save();
    QRect rect(this->width() - 75, 10 - m_DateTimeOffset, 65, 17);
    painter.setPen(getDateTimeColor());
    painter.setFont(getDateTimeFont());
    QString timestr;
    if (scheduleInfo()->allDay()) {
        timestr = ALL_DAY;
    } else {
        timestr = QString("%1-%2").arg(getScheduleBeginTime().toString("hh:mm")).arg(getScheduleEndTime().toString("hh:mm"));
    }
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop,
                     timestr);

    painter.restore();
}

void modifyScheduleItem::drawDate(QPainter &painter)
{
    painter.save();
    QRect rect(this->width() - 75, 27 - m_DateTimeOffset, 65, 17);
    painter.setPen(getDateTimeColor());
    painter.setFont(getDateTimeFont());
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop,
                     getShowDate().toString("yyyy/MM/dd"));
    painter.restore();
}

void modifyScheduleItem::setLineColor(const QColor &LineColor)
{
    m_LineColor = LineColor;
}

void modifyScheduleItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit signalSendSelectScheduleIndex(getIndex());
    }
}

QColor modifyScheduleItem::LineColor() const
{
    return m_LineColor;
}

void modifyScheduleItem::setTheMe(const int type)
{
    ItemWidget::setTheMe(type);
    QColor lineColor;
    if (type == 0 || type == 1) {
        lineColor = QColor("#000000");
        lineColor.setAlphaF(0.1);
    } else {
        //设置深色主题分割线颜色
        lineColor = QColor("#FFFFFF");
        lineColor.setAlphaF(0.1);
    }
    setLineColor(lineColor);
}
