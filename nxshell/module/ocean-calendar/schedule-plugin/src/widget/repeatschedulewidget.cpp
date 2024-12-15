// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "repeatschedulewidget.h"
#include "scheduleitemwidget.h"
#include "buttonwidget.h"
#include "../globaldef.h"

repeatScheduleWidget::repeatScheduleWidget(repeatScheduleWidget::Operation_type operation, repeatScheduleWidget::Widget_type widgetype, bool tocreateBtn, QWidget *parent)
    : IconDFrame(parent)
    , m_OperationType(operation)
    , m_WidgetType(widgetype)
    , m_createBtnBool(tocreateBtn)
{
}

void repeatScheduleWidget::setSchedule(const DSchedule::Ptr &info)
{
    m_scheduleInfo.clear();
    m_scheduleInfo.append(info);
    initUI();
}

void repeatScheduleWidget::initUI()
{
    m_scheduleitemwidget = new scheduleitemwidget(this);
    QVBoxLayout *mainlayout = new QVBoxLayout();
    m_scheduleitemwidget->setScheduleDtailInfo(m_scheduleInfo);
    m_scheduleitemwidget->addscheduleitem();
    mainlayout->addWidget(m_scheduleitemwidget);

    if (m_createBtnBool) {
        buttonwidget *button = new buttonwidget(this);
        m_buttonCount = 0;
        switch (m_WidgetType) {
        case Widget_Confirm: {
            button->addbutton(CANCEL_BUTTON_STRING);
            if (m_OperationType == Operation_Cancel) {
                button->addbutton(CONFIRM_BUTTON_STRING, true, buttonwidget::ButtonWarning);
            } else {
                button->addbutton(CONFIRM_BUTTON_STRING, true, buttonwidget::ButtonRecommend);
            }
        } break;
        default: {
            if (m_OperationType == Operation_Cancel) {
                button->addbutton(CANCEL_BUTTON_STRING);
                button->addbutton(DELETEALL_BUTTON_STRING);
                button->addbutton(ONLY_DELETE_THIS_BUTTON_STRING, true, buttonwidget::ButtonWarning);
            } else {
                button->addbutton(CANCEL_BUTTON_STRING);
                button->addbutton(CHANGE_ALL_BUTTON_STRING);
                button->addbutton(CHANGE_ONLYL_BUTTON_STRING, true, buttonwidget::ButtonRecommend);
            }
        } break;
        }
        m_buttonCount = button->buttonCount();
        connect(button, &buttonwidget::buttonClicked, this, &repeatScheduleWidget::slotButtonCheckNum);
        mainlayout->addSpacing(4);
        mainlayout->addWidget(button);
    }
    setCenterLayout(mainlayout);
}

void repeatScheduleWidget::slotButtonCheckNum(int index, const QString &text)
{
    emit signalButtonCheckNum(index, text, m_buttonCount);
}
