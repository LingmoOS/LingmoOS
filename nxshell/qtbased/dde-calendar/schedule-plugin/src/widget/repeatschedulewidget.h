// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REPEATSCHEDULEWIDGET_H
#define REPEATSCHEDULEWIDGET_H

#include "icondframe.h"
#include "dschedule.h"

class scheduleitemwidget;
class repeatScheduleWidget : public IconDFrame
{
    Q_OBJECT
public:
    enum Operation_type { Operation_Cancel,
                          Operation_Change
    };
    enum Widget_type { Widget_Repeat,
                       Widget_Confirm
    };

public:
    explicit repeatScheduleWidget(Operation_type operation, Widget_type widgetype, bool tocreateBtn = true, QWidget *parent = nullptr);
    void setSchedule(const DSchedule::Ptr &info);

private:
    void initUI();
signals:
    void signalButtonCheckNum(int index, const QString &text, const int buttonCount);
public slots:
    void slotButtonCheckNum(int index, const QString &text);

private:
    scheduleitemwidget *m_scheduleitemwidget {nullptr};
    DSchedule::List m_scheduleInfo;
    Operation_type m_OperationType {Operation_Cancel};
    Widget_type m_WidgetType {Widget_Confirm};
    int m_buttonCount {0};
    bool m_createBtnBool {true};
};

#endif // REPEATSCHEDULEWIDGET_H
