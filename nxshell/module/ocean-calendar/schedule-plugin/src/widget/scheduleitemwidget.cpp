// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleitemwidget.h"

#include "../globaldef.h"

#include <DPushButton>

#include <QMouseEvent>

scheduleitemwidget::scheduleitemwidget(QWidget *parent)
    : DWidget(parent)
{
}

scheduleitemwidget::~scheduleitemwidget()
{
}

void scheduleitemwidget::setScheduleDtailInfo(const DSchedule::List &scheduleInfo)
{
    m_scheduleInfo = scheduleInfo;
    sortScheduleWithTime();
}

void scheduleitemwidget::addscheduleitem()
{
    QVBoxLayout *mainlayout = new QVBoxLayout();
    scheduleitemdate *m_scheduleitemdate = new scheduleitemdate();
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(m_scheduleitemdate);
    mainlayout->addSpacing(10);
    mainlayout->setSpacing(1);

    if (m_scheduleInfo.count() == 1) {
        //单个日程
        scheduleitem *m_scheduleitem = new scheduleitem();
        connect(m_scheduleitem, &scheduleitem::signalItemPress, this, &scheduleitemwidget::signalItemPress);
        m_scheduleitem->setPositon(ItemWidget::ItemOnly);
        m_scheduleitemdate->setScheduleDtailInfo(m_scheduleInfo.first());
        m_scheduleitem->setScheduleInfo(m_scheduleInfo.first());
        mainlayout->addWidget(m_scheduleitem);
    } else if (m_scheduleInfo.count() > 1) {
        //多个日程
        for (int i = 0; i < m_scheduleInfo.count(); i++) {
            scheduleitem *m_scheduleitem = new scheduleitem();
            connect(m_scheduleitem, &scheduleitem::signalItemPress, this, &scheduleitemwidget::signalItemPress);
            if (i == 0) {
                m_scheduleitem->setPositon(ItemWidget::ItemTop);
            } else if (i == m_scheduleInfo.count() - 1) {
                m_scheduleitem->setPositon(ItemWidget::ItemBottom);
            } else {
                m_scheduleitem->setPositon(ItemWidget::ItemMiddle);
            }
            m_scheduleitemdate->setScheduleDtailInfo(m_scheduleInfo[i]);
            m_scheduleitem->setScheduleInfo(m_scheduleInfo[i]);
            mainlayout->addWidget(m_scheduleitem);
        }
    }

    this->setLayout(mainlayout);
}

void scheduleitemwidget::sortScheduleWithTime()
{
    for (int i = 0; i < m_scheduleInfo.count(); i++) {
        for (int j = 0; j < m_scheduleInfo.count() - i - 1; j++) {
            if (m_scheduleInfo[j]->dtStart() > m_scheduleInfo[j + 1]->dtStart())
                std::swap(m_scheduleInfo[j], m_scheduleInfo[j + 1]);
        }
    }
}

scheduleitem::scheduleitem(QWidget *parent)
    : ItemWidget(parent)
{
    setFixedHeight(36);
}

void scheduleitem::DrawItem(QPainter &painter)
{
    drawScheduleColorLine(painter);
    drawTime(painter);
    drawDetails(painter);
}

void scheduleitem::drawScheduleColorLine(QPainter &painter)
{
    painter.save();
    QRect rect(m_LineRightMargin,
               0,
               2,
               this->height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(ScheduleColor());
    painter.drawRect(rect);
    painter.restore();
}

void scheduleitem::drawTime(QPainter &painter)
{
    painter.save();
    QRect rect(m_timeLeftMargin, 0, 82, this->rect().height());
    painter.setPen(getDateTimeColor());
    painter.setFont(getDateTimeFont());
    QString timestr;
    if (scheduleInfo()->allDay()) {
        timestr = ALL_DAY;
    } else {
        timestr = QString("%1-%2").arg(scheduleInfo()->dtStart().toString("hh:mm")).arg(scheduleInfo()->dtEnd().toString("hh:mm"));
    }
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter,
                     timestr);
    painter.restore();
}

void scheduleitem::drawDetails(QPainter &painter)
{
    painter.save();
    QRect rect(m_DetailsRightMargin + m_LineRightMargin + 2, 0, m_DetailsWidth, this->rect().height());
    painter.setPen(getTitleColor());
    painter.setFont(getTitleFont());
    QFontMetrics metrics(getTitleFont());
    painter.drawText(rect,
                     Qt::AlignLeft | Qt::AlignVCenter,
                     metrics.elidedText(getTitleContent(), Qt::ElideRight, rect.width() - m_EllipsisWidth));
    painter.restore();
}

void scheduleitem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit signalItemPress(getScheduleInfo());
    }
}

scheduleitemdate::scheduleitemdate(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(20);
}

void scheduleitemdate::setScheduleDtailInfo(const DSchedule::Ptr &scheduelDtailInfo)
{
    m_scheduleDtailInfo = scheduelDtailInfo;
}

QColor scheduleitemdate::DetailsColor()
{
    int type = DGuiApplicationHelper::instance()->themeType();

    if (type == 0 || type == 1) {
        m_DateTimeColor.setRgb(0, 0, 0);
    } else {
        m_DateTimeColor.setRgb(255, 255, 255);
    }
    return m_DateTimeColor;
}

void scheduleitemdate::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(DetailsColor());
    painter.drawText(QRect(m_LeftMargin, 0, this->rect().width(), 20), Qt::AlignLeft | Qt::AlignVCenter,
                     QString("%1 %2").arg(m_scheduleDtailInfo->dtStart().date().toString("yyyy年MM月dd日")).arg(m_scheduleDtailInfo->dtStart().toString("dddd")));
    painter.restore();
}
