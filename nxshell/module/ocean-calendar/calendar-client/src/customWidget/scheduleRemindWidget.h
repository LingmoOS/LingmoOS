// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ScheduleRemindWidget_H
#define ScheduleRemindWidget_H

#include "dschedule.h"
#include "scheduledatamanage.h"

#include <DWidget>
#include <DArrowRectangle>
#include <DFrame>

DWIDGET_USE_NAMESPACE

class CenterWidget;
class ScheduleRemindWidget : public DArrowRectangle
{
    Q_OBJECT
    Q_DISABLE_COPY(ScheduleRemindWidget)
public:
    explicit ScheduleRemindWidget(QWidget *parent = nullptr);
    ~ScheduleRemindWidget() override;
    void setData(const DSchedule::Ptr &vScheduleInfo, const CSchedulesColor &gcolor);
    //设置箭头方向
    void setDirection(ArrowDirection value);
    void setTimeFormat(QString timeformat);
signals:

public slots:
private:
    CenterWidget *m_centerWidget = nullptr;
    DSchedule::Ptr m_ScheduleInfo;
    CSchedulesColor gdcolor;
};

class CenterWidget : public DFrame
{
    Q_OBJECT
public:
    explicit CenterWidget(DWidget *parent = nullptr);
    ~CenterWidget() override;
    void setData(const DSchedule::Ptr &vScheduleInfo, const CSchedulesColor &gcolor);
    void setTheMe(const int type = 0);
    void setTimeFormat(QString timeFormat = "h:mm");
private:
    void UpdateTextList();
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    QStringList testList;
    QFont textfont;
    int textwidth;
    int textheight;
    const int textRectWidth = 165;
    DSchedule::Ptr m_ScheduleInfo;
    CSchedulesColor gdcolor;
    QColor textColor;
    QColor timeColor;
    QString m_timeFormat;
};

#endif // ScheduleRemindWidget_H
