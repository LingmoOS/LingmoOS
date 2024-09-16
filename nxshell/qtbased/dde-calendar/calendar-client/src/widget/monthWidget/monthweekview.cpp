// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthweekview.h"
#include "scheduledatamanage.h"

#include <DPalette>

#include <QDebug>
#include <QDate>
#include <QPainter>
#include <QPainterPath>

DGUI_USE_NAMESPACE
CMonthWeekView::CMonthWeekView(QWidget *parent)
    : DWidget(parent)
{
    for (int i = 0 ; i < 7 ; ++i) {
        m_weekRect.append(new WeekRect());
    }
}

CMonthWeekView::~CMonthWeekView()
{
    for (int i = 0 ; i < 7 ; ++i) {
        WeekRect *weekRect =  m_weekRect.at(i);
        delete weekRect;
    }
    m_weekRect.clear();
}

void CMonthWeekView::setFirstDay(const Qt::DayOfWeek weekday)
{
    m_firstWeek = weekday;
    updateWeek();
}

void CMonthWeekView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_backgroundColor = "#E6EEF2";

    } else if (type == 2) {
        m_backgroundColor = "#82AEC1";
        m_backgroundColor.setAlphaF(0.10);
    }
    for (int i = 0 ; i < m_weekRect.size(); ++i) {
        m_weekRect.at(i)->setTheMe(type);
    }
}

void CMonthWeekView::updateWeek()
{
    Qt::DayOfWeek _setWeek;
    bool _showLine{false};
    for (int i = 0; i < m_weekRect.size(); ++i) {
        int weekNum = (m_firstWeek + i) % 7;
        _setWeek = static_cast<Qt::DayOfWeek>(weekNum == 0 ? 7 : weekNum);
        //如果为当前时间所在周则绘制横线
        if (_setWeek == m_currentWeek) {
            _showLine = true;
        } else {
            _showLine = false;
        }
        m_weekRect.at(i)->setWeek(_setWeek, _showLine);
    }
    update();
}

/**
 * @brief CMonthWeekView::setCurrentDate        设置当前时间,获取当前时间所在周
 * @param currentDate
 */
void CMonthWeekView::setCurrentDate(const QDate &currentDate)
{
    m_currentWeek = static_cast<Qt::DayOfWeek>(currentDate.dayOfWeek());
    updateWeek();
}

void CMonthWeekView::resizeEvent(QResizeEvent *event)
{
    qreal weekRectWith = width() / 7;
    QRectF _rectF;
    for (int i = 0 ; i < m_weekRect.size(); ++i) {
        _rectF.setRect(i * weekRectWith, 0, weekRectWith, this->height());
        m_weekRect.at(i)->setRect(_rectF);
    }
    DWidget::resizeEvent(event);
}

void CMonthWeekView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath painterPath;
    //从左下角开始
    painterPath.moveTo(0, this->height());
    painterPath.lineTo(this->width(), this->height());
    painterPath.lineTo(this->width(), m_radius);
    painterPath.arcTo(QRect(this->width() - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
    painterPath.lineTo(m_radius, 0);
    painterPath.arcTo(QRect(0, 0, m_radius * 2, m_radius * 2), 90, 90);
    painter.setBrush(m_backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
    for (int i = 0 ; i < m_weekRect.size(); ++i) {
        m_weekRect.at(i)->paintRect(painter);
    }
    painter.end();
}

WeekRect::WeekRect()
    : m_showLine(false)
{
    m_font.setWeight(QFont::Medium);
    m_font.setPixelSize(DDECalendar::FontSizeSixteen);
}

void WeekRect::setWeek(const Qt::DayOfWeek &showWeek, const bool &showLine)
{
    m_showWeek = showWeek;
    m_showLine = showLine;
    QLocale locale;
    m_weekStr = locale.dayName(m_showWeek, QLocale::ShortFormat);
}

void WeekRect::setRect(const QRectF &rectF)
{
    m_rectF = rectF;
}

void WeekRect::paintRect(QPainter &painter)
{
    //绘制文字
    painter.save();
    painter.setFont(m_font);
    if (m_showWeek > 5) {
        painter.setPen(m_activeColor);
    } else {
        painter.setPen(m_testColor);
    }
    painter.drawText(m_rectF, Qt::AlignCenter, m_weekStr);
    painter.restore();
    if (m_showLine) {
        //绘制横线
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_activeColor);
        painter.drawRect(QRectF(m_rectF.x(), m_rectF.height() - m_lineHeight, m_rectF.width(), m_lineHeight));
        painter.restore();
    }

}

void WeekRect::setTheMe(int type)
{
    m_activeColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    if (type == 0 || type == 1) {
        m_testColor = "#6F6F6F";
    } else {
        m_testColor = "#C0C6D4";
    }
}
