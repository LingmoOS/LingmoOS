// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef YEARSCHEDULEVIEW_H
#define YEARSCHEDULEVIEW_H

#include "dschedule.h"

#include <DWidget>
#include <DArrowRectangle>

DWIDGET_USE_NAMESPACE
class CYearScheduleOutView;
class CYearScheduleView : public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief CYearScheduleView 构造函数
     * @param parent 父类
     */
    explicit CYearScheduleView(QWidget *parent = nullptr);
    /**
      * @brief ~CYearScheduleView 析构函数
      */
    ~CYearScheduleView() override;
    /**
     * @brief setData 设置日程信息，并全天日程置于非全天日程之前
     * @param vListData 日程信息
     */
    void setData(DSchedule::List &vListData);
    /**
     * @brief getlistdate 获取日程信息
     * @return  日程信息
     */
    DSchedule::List getlistdate()
    {
        return m_vlistData;
    }
    /**
     * @brief clearData 清除日程信息
     */
    void clearData();
    /**
     * @brief showWindow 设置日程浮框的大小
     * @return 日程浮框的宽度
     */
    void showWindow();
    /**
     * @brief setTheMe 根据系统主题类型设置颜色
     * @param type 系统主题类型
     */
    void setTheMe(int type = 0);
    /**
     * @brief setCurrentDate 设置日程所在当天的日期
     * @param cDate 日期
     */
    void setCurrentDate(const QDate &cdate);
    /**
     * @brief getCurrentDate 获取日程所在当天的日期
     * @return 日期
     */
    QDate getCurrentDate();

    void setTimeFormat(const QString &format = "h:mm");
    //判断点击位置是否包含对应的日程信息
    int getPressScheduleIndex();

private:
    /**
     * @brief updateDateShow 调整最多展示日程为五个，并设置浮框大小
     */
    void updateDateShow();
protected:
    /**
     * @brief paintEvent 绘制日程
     * @param event 绘图事件
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief paintItem 绘制日程
     * @param info 日程信息
     * @param index 日程的索引
     */
    void paintItem(QPainter &painter, DSchedule::Ptr info, int index);
    /**
     * @brief paintItem
     */
    void paintItem(QPainter &painter);

private:
    DSchedule::List m_vlistData;
    QDate m_currentDate;
    QColor m_btimecolor = "#526A7F";
    QColor m_btTextColor = "#414D68";
    QFont m_textfont;
    QString m_timeFormat = "h:mm";
    QVector<QRect> m_drawRect;
};

class CYearScheduleOutView : public DArrowRectangle
{
    Q_OBJECT
public:
    /**
     * @brief CYearScheduleOutView 构造函数
     * @param parent 父类
     */
    explicit CYearScheduleOutView(QWidget *parent = nullptr);
    /**
     * @brief setData 设置日程信息
     * @param vListData 日程信息
     */
    void setData(DSchedule::List &vListData);
    /**
     * @brief clearData 清楚数据
     */
    void clearData();
    /**
     * @brief setTheMe 设置系统主题颜色
     * @param type 系统主题
     */
    void setTheMe(int type = 0);
    /**
     * @brief setCurrentDate 设置日程所在当天的日期
     * @param cDate 日期
     */
    void setCurrentDate(const QDate &cDate);
    //设置箭头方向
    void setDirection(ArrowDirection value);
    void setTimeFormat(const QString &format);
signals:
    /**
     * @brief signalsViewSelectDate 跳转视图信号
     * @param date 当天的时间
     */
    void signalsViewSelectDate(QDate date, const int pressType = 3);
    /**
     * @brief signalupdateschedule 更新日程信息的信号
     */
    void signalupdateschedule();
private:
    CYearScheduleView *yearscheduleview = nullptr;
    DSchedule::List scheduleinfoList;
    QDate currentdate;
    int list_count = 0;
protected:
    /**
     * @brief mousePressEvent 鼠标单击事件，单击非节日日程进行编辑，单击"..."区域跳转到周视图。
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;
};
#endif // YEARSCHEDULEVIEW_H
