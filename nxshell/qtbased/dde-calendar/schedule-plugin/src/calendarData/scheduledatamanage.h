// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEDATAMANAGE_H
#define SCHEDULEDATAMANAGE_H
#include "dschedule.h"
#include "dscheduletype.h"

#include <DGuiApplicationHelper>

#include <QThread>
#include <QDate>
#include <QMutex>

DGUI_USE_NAMESPACE
struct CSchedulesColor {
    QColor normalColor; //正常状态颜色
    QColor hoverColor; //鼠标悬浮颜色
    QColor pressColor; //鼠标点击颜色
    QColor hightColor; //高亮色
    QColor orginalColor; //最初的颜色
};

class CScheduleDataManage : public QObject
{
    Q_OBJECT
public:
    static CScheduleDataManage *getScheduleDataManage();
    //根据日程类型ID获取颜色信息
    CSchedulesColor getScheduleColorByType(const QString &type);
    static QColor getSystemActiveColor();
    static QColor getTextColor();
    void setTheMe(int type = 0);
    int getTheme() const
    {
        return m_theme;
    }

private:
    CScheduleDataManage(QObject *parent = nullptr);
    ~CScheduleDataManage();

private:
    int m_theme = 0;
    static CScheduleDataManage *m_vscheduleDataManage;
};
#endif // SCHEDULEVIEW_H
