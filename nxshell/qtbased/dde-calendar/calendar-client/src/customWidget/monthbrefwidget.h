// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHBREFWIDGET_H
#define MONTHBREFWIDGET_H

#include "cschedulebasewidget.h"
#include <QWidget>
#include <QPushButton>
#include <QDate>
#include <QSet>

class CMonthDayRectWidget;

class MonthBrefWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MonthBrefWidget(QWidget *parent = nullptr);
    ~MonthBrefWidget() override;

    //当月所有日期共有数据
    struct GlobalData {

    private:
        QDate m_showMonthDate;
        static QDate m_selectedMonth; //选中的月
        static QDate m_selectedDate; //选中的日期
        bool m_scheduleDateFlag[32] = {false};  //是否有日程
        bool m_searchedDateFlag[32] = {false};  //是否有选中日程
    public:
        //设置选中的日期
        void setSelectedDate(const QDate& date) {
            m_selectedDate = date;
            m_selectedMonth = m_showMonthDate;
        }

        //判断是否是选中的日期
        bool isSelectedDate(const QDate& date) {
            //当全局日期不等于此类静态日期时（其他界面更改日期的情况下），采用全局时间做判断
            if (CScheduleBaseWidget::getSelectDate() != m_selectedDate) {
                return  (isBelongMonth(date) && date == CScheduleBaseWidget::getSelectDate());
            }
            //用此类静态日期做判断（内部界面操作）
            return m_showMonthDate == m_selectedMonth && date == m_selectedDate;
        }

        //判断日期是否是本月内的日期
        bool isBelongMonth(const QDate& date) {
            return  (m_showMonthDate.year() == date.year() && m_showMonthDate.month() == date.month());
        }

        //判断该日期是否含有日程
        bool isHasScheduleByDate(const QDate& date) {
            if (isBelongMonth(date)) {
                return m_scheduleDateFlag[date.day()];
            }
            return false;
        }

        //判断该日期是否含有被搜索的日程
        bool isHasSearchedByDate(const QDate& date) {
            if (isBelongMonth(date)) {
                return m_searchedDateFlag[date.day()];
            }
            return false;
        }

        friend MonthBrefWidget;
    };

    //设置显示的月
    void setShowMonthDate(const QDate& monthDate);
    //设置含有日程的日期集合
    void setHasScheduleDateSet(const QSet<QDate> &hasScheduleSet);
    //设置含有搜索日程的日期集合
    void setHasSearchScheduleSet(const QSet<QDate> &hasScheduleSet);

protected:
    //鼠标按下事件
    void mousePressEvent(QMouseEvent *event) override;
    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;
    //鼠标移动事件，设置hover状态
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    /**
     * @brief signalPressDate 鼠标点击日期的信号
     * @param date 传递时间参数
     */
    void signalPressDate(const QDate &date);
    /**
     * @brief signalDoubleClickDate 鼠标双击日期的信号
     * @param date 传递时间参数
     */
    void signalDoubleClickDate(const QDate &date);
private:
    QVector<CMonthDayRectWidget *>        m_DayItem;
    GlobalData *m_globalData = nullptr;

    //触摸状态 0：原始  1：点击  2：移动
    int         m_touchState{0};
    //触摸点击坐标
    QPoint      m_touchBeginPoint;
};

/**
 * @brief The CMonthDayRectWidget class
 * 单个日期显示控件
 */
class CMonthDayRectWidget : public QPushButton
{
    Q_OBJECT
public:

    explicit CMonthDayRectWidget(MonthBrefWidget::GlobalData* globalData, QWidget *parent = nullptr);
    //设置当天的时间
    void setDate(const QDate &date);
    //获取当天的时间
    QDate getDate()const;
    //系统主题变化
    void setTheMe(int type = 0);

signals:
    /**
     * @brief signalPressDate 鼠标点击日期的信号
     * @param date 传递时间参数
     */
    void signalClicked(const QDate &date);
    /**
     * @brief signalDoubleClick 鼠标双击日期的信号
     * @param date 传递时间参数
     */
    void signalDoubleClick(const QDate &date);

protected:
    //鼠标按下事件
    void mousePressEvent(QMouseEvent *event) override;
    //鼠标双击事件
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;
    //绘制日期以及当天状态
    void paintEvent(QPaintEvent *event) override;
    //鼠标进入事件
    void enterEvent(QEvent *event) override;
    //离开事件，设置当前选中的日期为空
    void leaveEvent(QEvent *event) override;

private:
    QDate m_date;       //当前位置显示的日期
    bool m_pressed = false;     //按下状态
    bool m_hovered = false;     //悬浮状态
    QColor m_ceventColor = "#FF5D00";   //日程圆点颜色

    MonthBrefWidget::GlobalData* m_globaldata = nullptr;
};


#endif // MONTHBREFWIDGET_H
