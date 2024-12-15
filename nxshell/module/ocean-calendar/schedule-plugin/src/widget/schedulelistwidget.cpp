// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulelistwidget.h"
#include <QVBoxLayout>

#include "modifyscheduleitem.h"
#include "opencalendarwidget.h"
#include "../globaldef.h"

scheduleListWidget::scheduleListWidget(QWidget *parent)
    : IconDFrame(parent)
{
}

void scheduleListWidget::setScheduleInfoVector(const DSchedule::List &ScheduleInfoVector)
{
    m_ScheduleInfoVector = ScheduleInfoVector;
    updateUI();
}

void scheduleListWidget::updateUI()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(1);
    bool showOpenWidget = m_ScheduleInfoVector.size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_ScheduleInfoVector.size();
    for (int i = 0; i < showcount; ++i) {
        modifyScheduleItem *item = new modifyScheduleItem();
        if (i == 0) {
            item->setPositon(ItemWidget::ItemTop);
        } else if (i == showcount - 1) {
            item->setPositon(ItemWidget::ItemBottom);
        } else {
            item->setPositon(ItemWidget::ItemMiddle);
        }
        item->setIndex(i + 1);
        item->setScheduleInfo(m_ScheduleInfoVector[i]);
        connect(item, &modifyScheduleItem::signalSendSelectScheduleIndex,
                this, &scheduleListWidget::signalSelectScheduleIndex);
        layout->addWidget(item);
    }

    if (showOpenWidget) {
        OpenCalendarWidget *openWidget = new OpenCalendarWidget();
        openWidget->setScheduleCount(m_ScheduleInfoVector.size());
        layout->addSpacing(6);
        layout->addWidget(openWidget);
    }
    this->setCenterLayout(layout);
}
