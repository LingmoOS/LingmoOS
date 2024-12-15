// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHSCHEDULEVIEW_H
#define MONTHSCHEDULEVIEW_H

#include "dschedule.h"
#include "scheduledaterangeinfo.h"
#include "graphicsItem/draginfoitem.h"
#include "graphicsItem/cmonthscheduleitem.h"

#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

DWIDGET_USE_NAMESPACE

class CWeekScheduleView;
class CMonthScheduleView : public QObject
{
    Q_OBJECT
public:
    explicit CMonthScheduleView(QWidget *parent, QGraphicsScene *scene);
    ~CMonthScheduleView() override;
    void setallsize(int w, int h, int left, int top, int buttom, int itemHeight = 22);
    void setData(QMap<QDate, DSchedule::List> &data, int currentMonth);
    void updateData();
    void updateHeight();
    QVector<QGraphicsRectItem *> getScheduleShowItem() const;
    int getScheduleHeight() const
    {
        return m_ItemHeight;
    }
    void updateDate(const DSchedule::Ptr &info);
    void changeDate(const DSchedule::Ptr &info);
    void updateDate(const int row, const DSchedule::Ptr &info);
signals:
    void signalsUpdateSchedule(int id = 0);
    void signalsCurrentScheduleDate(QDate date);
    void signalUpdateUI(int type);
    void signalPressScheduleShow(const bool isShow, const DSchedule::Ptr &out = DSchedule::Ptr());
public slots:
    void slotFontChange();
private:
    void updateDateShow(QVector<QVector<MScheduleDateRangeInfo>> &vCMDaySchedule, QVector<QGraphicsRectItem *> &scheduleShowItem);
    void createScheduleItemWidget(MScheduleDateRangeInfo info, int cNum, QVector<QGraphicsRectItem *> &scheduleShowItem);
    void createScheduleNumWidget(MScheduleDateRangeInfo info, int cNum, QVector<QGraphicsRectItem *> &scheduleShowItem);
    void computePos(int cNum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh);

private:
    QMap<QDate, DSchedule::List> m_data;
    int m_cNum = 2; //日程层数
    int m_currentMonth = 0;
    QDate m_beginDate;
    QDate m_endDate;
    int m_width = 0;
    int m_height = 0;
    int m_leftMargin = 0;
    int m_topMargin = 0;
    int m_bottomMargin = 0;
    QVector<CWeekScheduleView *> m_weekSchedule;
    int m_ItemHeight = 22;
    QGraphicsScene *m_Scene = nullptr;
};

class CWeekScheduleView : public QObject
{
    Q_OBJECT
    typedef QVector<MScheduleDateRangeInfo> RowScheduleInfo;

public:
    explicit CWeekScheduleView(QObject *parent = nullptr);
    ~CWeekScheduleView() override;

public:
    void setData(QMap<QDate, DSchedule::List> &data, const QDate &startDate, const QDate &stopDate);
    bool addData(const DSchedule::Ptr &info);
    void changeDate(const DSchedule::Ptr &info);
    void setHeight(const int ScheduleHeight, const int dayHeight);
    QVector<RowScheduleInfo> getMScheduleInfo() const
    {
        return m_MScheduleInfo;
    }
    QVector<QGraphicsRectItem *> &getScheduleShowItem()
    {
        return m_scheduleShowItem;
    }
    void updateSchedule(const bool isNormalDisplay, const DSchedule::Ptr &info = DSchedule::Ptr());
    void clearItem();
private:
    void setMaxNum();
    void mScheduleClear();
    void sortAndFilter(QVector<MScheduleDateRangeInfo> &vMDaySchedule);
    /**
     * @brief addShowSchedule       添加显示的日程
     * @param startPos              其实位置
     * @param endPos                结束位置
     * @param addRow                需要添加的行
     * @param addInfo               添加的日程
     */
    void addShowSchedule(const int &startPos, const int &endPos, const int &addRow, const DSchedule::Ptr &addInfo);

private:
    QVector<QGraphicsRectItem *> m_scheduleShowItem;
    QVector<RowScheduleInfo> m_MScheduleInfo;
    DSchedule::List m_ScheduleInfo;
    QVector<int> m_ColumnScheduleCount;
    int m_ScheduleHeight = 0;
    int m_DayHeight = 0;
    int m_MaxNum = 0;
    QDate beginDate;
    QDate endDate;
    int m_colum = 0;
};
#endif // MONTHSCHEDULEVIEW_H
