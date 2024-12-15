// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "itemwidget.h"

#include <DGuiApplicationHelper>
#include <QPainterPath>

ItemWidget::ItemWidget(QWidget *parent)
    : DWidget(parent)
{
    QFont datetimefont;
    datetimefont.setPixelSize(12);
    QFont titlefont;
    titlefont.setPixelSize(14);
    setTitleFont(titlefont);
    setDateTimeFont(datetimefont);
}

void ItemWidget::paintEvent(QPaintEvent *event)
{
    setTheMe(DGuiApplicationHelper::instance()->themeType());
    DWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    drawBackground(painter);
    DrawItem(painter);
}

void ItemWidget::drawBackground(QPainter &painter)
{
    const int radius = 8;
    const int diameter = radius * 2;
    const int RectWidth = this->rect().width();
    const int RectHeight = this->rect().height();
    painter.save();
    painter.setPen(Qt::NoPen);
    QColor backgroundcolor(getBackgroundColor());
    painter.setBrush(backgroundcolor);
    switch (m_Positon) {
    case ItemTop: {
        QPainterPath painterPath;
        painterPath.moveTo(0, RectHeight);
        painterPath.lineTo(RectWidth, RectHeight);
        painterPath.lineTo(RectWidth, diameter);
        painterPath.arcTo(QRect(RectWidth - diameter, 0, diameter, diameter), 0, 90);
        painterPath.lineTo(diameter, 0);
        painterPath.arcTo(QRect(0, 0, diameter, diameter), 90, 90);
        painterPath.lineTo(0, RectHeight);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
    } break;
    case ItemMiddle:
        painter.drawRect(this->rect());
        break;
    case ItemBottom: {
        QPainterPath painterPath;
        painterPath.moveTo(0, 0);
        painterPath.lineTo(0, RectHeight - diameter);
        painterPath.arcTo(QRect(0, RectHeight - diameter, diameter, diameter), 180, 90);
        painterPath.lineTo(RectWidth - diameter, RectHeight);
        painterPath.arcTo(QRect(RectWidth - diameter, RectHeight - diameter, diameter, diameter), 270, 90);
        painterPath.lineTo(RectWidth, 0);
        painterPath.lineTo(0, 0);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
    } break;
    case ItemOnly:
        painter.drawRoundedRect(this->rect(), 8, 8);
        break;
    }
    painter.restore();
}

QColor ItemWidget::getBackgroundColor()
{
    return m_BackgroundColor;
}

void ItemWidget::setTheMe(const int type)
{
    QColor titleColor;
    QColor datetimeColor;
    if (type == 0 || type == 1) {
        titleColor.setRgb(0, 0, 0);
        titleColor.setAlphaF(0.8);

        datetimeColor.setRgb(0, 0, 0);
        datetimeColor.setAlphaF(0.6);
        //设置浅色背景色
        m_BackgroundColor = "#000000";
        m_BackgroundColor.setAlphaF(0.05);
    } else {
        titleColor.setRgb(255, 255, 255);
        titleColor.setAlphaF(1);

        datetimeColor.setRgb(255, 255, 255);
        datetimeColor.setAlphaF(0.6);
        //设置深色背景色
        m_BackgroundColor = "#FFFFFF";
        m_BackgroundColor.setAlphaF(0.05);
    }
    setTitleColor(titleColor);
    setDateTimeColor(datetimeColor);
}

QColor ItemWidget::ScheduleColor()
{
    return CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_scheduleInfo->scheduleTypeID()).orginalColor;
}

DSchedule::Ptr ItemWidget::getScheduleInfo() const
{
    return m_scheduleInfo;
}

void ItemWidget::setDateTimeFont(const QFont &DateTimeFont)
{
    m_DateTimeFont = DateTimeFont;
}

void ItemWidget::setTitleFont(const QFont &TitleFont)
{
    m_TitleFont = TitleFont;
}

void ItemWidget::setDateTimeColor(const QColor &DateTimeColor)
{
    m_DateTimeColor = DateTimeColor;
}

DSchedule::Ptr ItemWidget::scheduleInfo() const
{
    return m_scheduleInfo;
}

void ItemWidget::setScheduleInfo(const DSchedule::Ptr &scheduleInfo)
{
    m_scheduleInfo = scheduleInfo;
    setScheduleBeginTime(scheduleInfo->dtStart());
    setScheduleEndTime(scheduleInfo->dtEnd());
    setShowDate(scheduleInfo->dtStart().date());
    setTitleContent(scheduleInfo->summary());
}

void ItemWidget::setTitleColor(const QColor &TitleColor)
{
    m_TitleColor = TitleColor;
}

QColor ItemWidget::getDateTimeColor() const
{
    return m_DateTimeColor;
}

QFont ItemWidget::getDateTimeFont() const
{
    return m_DateTimeFont;
}

QColor ItemWidget::getTitleColor() const
{
    return m_TitleColor;
}

QFont ItemWidget::getTitleFont() const
{
    return m_TitleFont;
}

QDateTime ItemWidget::getScheduleEndTime() const
{
    return m_ScheduleEndTime;
}

void ItemWidget::setScheduleEndTime(const QDateTime &ScheduleEndTime)
{
    m_ScheduleEndTime = ScheduleEndTime;
}

QDateTime ItemWidget::getScheduleBeginTime() const
{
    return m_ScheduleBeginTime;
}

void ItemWidget::setScheduleBeginTime(const QDateTime &ScheduleBeginTime)
{
    m_ScheduleBeginTime = ScheduleBeginTime;
}

QDate ItemWidget::getShowDate() const
{
    return m_ShowDate;
}

void ItemWidget::setShowDate(const QDate &ShowDate)
{
    m_ShowDate = ShowDate;
}

QString ItemWidget::getTitleContent() const
{
    return m_TitleContent;
}

void ItemWidget::setTitleContent(const QString &TitleContent)
{
    m_TitleContent = TitleContent;
}

void ItemWidget::setPositon(const Item_Position &Positon)
{
    m_Positon = Positon;
}

void ItemWidget::DrawItem(QPainter &painter)
{
    Q_UNUSED(painter);
}
