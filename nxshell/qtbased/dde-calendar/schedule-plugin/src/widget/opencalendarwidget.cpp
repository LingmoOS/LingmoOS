// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "opencalendarwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QProcess>

#include "../globaldef.h"

#include <DApplicationHelper>

OpenCalendarWidget::OpenCalendarWidget(QWidget *parent)
    : DWidget(parent)
{
    setFixedHeight(OPENCALENDAR_WDIGET_HEIGHT);
}

void OpenCalendarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    setTheMe(DApplicationHelper::instance()->themeType());
    QPainter painter(this);
    painter.setPen(TitleColor());
    painter.setFont(TitleFont());
    painter.drawText(this->rect(),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString(OPENCALENDAR_WIDGET_TITLE).arg(m_ScheduleCount));
}

void OpenCalendarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QProcess proc;
        proc.startDetached(PROCESS_OPEN_CALENDAR);
    }
}

QColor OpenCalendarWidget::TitleColor() const
{
    return m_TitleColor;
}

void OpenCalendarWidget::setTitleColor(const QColor &TitleColor)
{
    m_TitleColor = TitleColor;
}

QFont OpenCalendarWidget::TitleFont()
{
    m_TitleFont.setPixelSize(12);
    return m_TitleFont;
}

void OpenCalendarWidget::setTheMe(const int type)
{
    QColor titleColor;
    if (type == 0 || type == 1) {
        titleColor.setRgb(0, 0, 0);
        titleColor.setAlphaF(0.4);
    } else {
        titleColor.setRgb(255, 255, 255);
        titleColor.setAlphaF(0.4);
    }
    setTitleColor(titleColor);
}

void OpenCalendarWidget::setScheduleCount(int ScheduleCount)
{
    m_ScheduleCount = ScheduleCount;
    update();
}
