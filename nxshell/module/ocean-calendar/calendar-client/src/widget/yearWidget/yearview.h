// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef YEARVIEW_H
#define YEARVIEW_H

#include "customframe.h"
#include "monthbrefwidget.h"
#include "scheduledatamanage.h"
#include "cweekwidget.h"

#include <DWidget>

#include <QList>
#include <QDate>
#include <QLabel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class CYearMonthView;
class CYearView: public CustomFrame
{
    Q_OBJECT
public:
    explicit CYearView(QWidget *parent = nullptr);
    //根据系统主题类型，设置颜色
    void setTheMe(int type = 0);
    //设置显示的月
    void setShowMonthDate(const QDate &showMonth);
    //获取显示的月
    QDate getShowMonthDate();
    //设置含有日程的日期集合
    void setHasScheduleSet(const QSet<QDate> &hasScheduleSet);
    //设置含有搜索日程的日期集合
    void setHasSearchScheduleSet(const QSet<QDate> &hasScheduleSet);
    //获取这个月开始结束时间
    bool getStartAndStopDate(QDate &startDate, QDate &stopDate);
signals:
    /**
     * @brief signalMousePress      鼠标点击事件触发信号
     * @param selectDate            选择时间
     * @param pressType             触发事件类型
     *          0:点击时间  1:双击时间  2: 双击月
     */
    void signalMousePress(const QDate &selectDate, const int pressType = 0);
public slots:

    void slotDoubleClickDate(const QDate &date);
    //鼠标单击日期，显示日程浮框
    void slotPressClickDate(const QDate &date);
protected:
    //过滤器，双击年视图下的月份跳转到月视图。
    bool eventFilter(QObject *o, QEvent *e) override;
    //更新月份框的高度
    void resizeEvent(QResizeEvent *event) override;
    //绘制每个月的背景
    void paintEvent(QPaintEvent *e) override;

private:
    CustomFrame         *m_currentMouth = nullptr;
    QVector<QDate>      m_days{};
    QDate               m_showMonthDate;
    QFont               m_momthFont;
    QColor              m_currentDayTextColor = "#2ca7f8";
    QColor              m_weekendsTextColor = Qt::black;
    QColor              m_festivalTextColor = Qt::black;
    QColor              m_cellBackgroundColor = "#FFFFFF" ;
    QVBoxLayout         *m_hhLayout = nullptr;
    int                 m_themetype = 1;
    QColor              m_bnormalColor = "#FFFFFF";
    const int           m_radius = 8;
    const int           m_borderframew = 0;
    MonthBrefWidget     *m_monthView = nullptr;
    CWeekWidget         *m_weekWidget = nullptr; //周显示区域
};
#endif // YEARVIEW_H
