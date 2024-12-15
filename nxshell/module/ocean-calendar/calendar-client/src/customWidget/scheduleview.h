// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEVIEW_H
#define SCHEDULEVIEW_H
#include "dschedule.h"
#include "scheduleRemindWidget.h"
#include "../widget/touchgestureoperation.h"
#include "graphicsview.h"

#include <DFrame>

#include <QDate>

DWIDGET_USE_NAMESPACE
#define ScheduleViewPos CWeekDayGraphicsview::ViewPosition
class CAllDayEventWeekView;
class QVBoxLayout;
class CScheduleView : public DFrame
{
    Q_OBJECT
public:
    CScheduleView(QWidget *parent = nullptr, ScheduleViewPos viewType = ScheduleViewPos::WeekPos);
    ~CScheduleView() override;
    void setviewMargin(int left, int top, int right, int bottom);
    void setRange(int w, int h, QDate begin, QDate end);
    void setRange(QDate begin, QDate end);
    void setTheMe(int type = 0);
    void setLunarVisible(bool state);
    void setTime(QTime time);
    void setSelectSchedule(const DSchedule::Ptr &scheduleInfo);
    void updateHeight();
    bool IsDragging();
    void setCurrentDate(const QDateTime &currentDate);
    //设置显示日程
    void setShowScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo);
    void setTimeFormat(QString timeformat);
signals:
    void signalsCurrentScheduleDate(QDate date);
    /**
     * @brief signalAngleDelta      发送滚动信号滚动相对量
     * @param delta     滚动相对量
     */
    void signalAngleDelta(int delta);
    void signalSwitchPrePage();
    void signalSwitchNextPage();
public slots:
    void slotPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType = 0);
    void setDate(QDate date);
    void slotupdateSchedule();
    //快捷键删除日程
    void slotDeleteitem();
    void slotCurrentScheduleDate(QDate date);
    void slotScheduleShow(const bool isShow, const DSchedule::Ptr &out = DSchedule::Ptr());
    void slotUpdatePaint(const int topM);
    void slotUpdateScene();
    //焦点切换到某个视图
    void slotSwitchView(const QDate &focusDate, CWeekDayGraphicsview::ViewType viewtype, bool setItemFocus);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;
    bool event(QEvent *e) override;
private:
    void initUI();
    void initConnection();
    void updateSchedule();
    void updateAllday();
    int scheduleViewHeight();

private:
    CGraphicsView *m_graphicsView = nullptr;
    QVector<int> m_vPos;
    QVector<int> m_vHours;
    CAllDayEventWeekView *m_alldaylist = nullptr;
    QMap<QDate, DSchedule::List> m_showSchedule {};
    int m_leftMargin;
    int m_topMargin;
    int m_rightMargin = 0;
    qint64 m_TotalDay;
    QDate m_currteDate;
    QDate m_beginDate;
    QDate m_endDate;
    // 0: 周  1:日
    //    int m_viewType = 0;
    ScheduleViewPos m_viewPos;
    int m_sMaxNum = 4;
    QColor m_linecolor = Qt::lightGray;
    QColor m_ALLDayColor = "#303030";
    QColor m_timeColor = "#7D7D7D";
    QColor m_currenttimecolor = "#F74444";
    int m_currentTimeType = 0;
    QVBoxLayout *m_layout = nullptr;
    ScheduleRemindWidget *m_ScheduleRemindWidget = nullptr;
    int m_minTime; //最小高度对应的最小时间
    QFont font;
    const int m_radius = 8;
    /**
     * @brief m_touchGesture        触摸手势处理
     */
    touchGestureOperation m_touchGesture;
    QColor              m_outerBorderColor;             //外边框背景色
    QString m_timeFormat;
};

#endif // SCHEDULEVIEW_H
