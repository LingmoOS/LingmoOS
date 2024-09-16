// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include "dschedule.h"
#include "draginfographicsview.h"
#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cweekdaygraphicsview.h"

#include <DMenu>

#include <QGraphicsScene>
#include <QAction>
#include <QMutex>
#include <QDrag>

DWIDGET_USE_NAMESPACE

typedef struct _tagScheduleclassificationInfo {
    QDateTime begindate;
    QDateTime enddate;
    DSchedule::List vData;

} ScheduleclassificationInfo;

class CScheduleCoorManage;
class CScheduleItem;
class CGraphicsView : public CWeekDayGraphicsview
{
    Q_OBJECT
public:
    explicit CGraphicsView(QWidget *parent, ViewPosition Type = WeekPos);
    ~CGraphicsView() override;
    void setMargins(int left, int top, int right, int bottom);
    QMargins getMargins()
    {
        return m_margins;
    }
    void updateHeight();
    void setCurrentDate(const QDateTime &currentDate);
    void setInfo(const DSchedule::List &info);
    void addScheduleItem(const DSchedule::Ptr &info, QDate date, int index, int totalNum, int type, int viewtype, int maxnum);
    //设置搜索选中日程
    void setSelectSearchSchedule(const DSchedule::Ptr &info) override;
    void clearSchedule();

    void setMinTime(const int &minTime)
    {
        m_minTime = minTime;
    }
    void setMaxNum(const int maxnum)
    {
        m_sMaxNum = maxnum;
    }
    void keepCenterOnScene();

    void scheduleClassificationType(DSchedule::List &scheduleInfolist, QList<ScheduleclassificationInfo> &info);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event) override;
#endif
    void resizeEvent(QResizeEvent *event) override;
    void setTime(QTime time);
    void updateInfo() override;
protected:
    void paintEvent(QPaintEvent *event) override;
public slots:
    void scrollBarValueChangedSlot();
    void slotDoubleEvent(int type);
    void slotScrollBar();
    void slotUpdateScene();
signals:
    void signalsPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType = 0);
    void signalsCurrentScheduleDate(QDate date);
private:
    void TimeRound(QDateTime &dtime);
    QDateTime TimeRounding(const QDateTime &time);
    void centerOnScene(const QPointF &pos);
    void setSceneHeightScale(const QPointF &pos);
public:
    void setTheMe(int type = 0) override;

protected:
    void slotCreate(const QDateTime &date) override;
    bool MeetCreationConditions(const QDateTime &date) override;
    void upDateInfoShow(const DragStatus &status = NONE, const DSchedule::Ptr &info = DSchedule::Ptr()) override;
    QDateTime getPosDate(const QPoint &p) override;
    void ShowSchedule(DragInfoItem *infoitem) override;
    void MoveInfoProcess(DSchedule::Ptr &info, const QPointF &pos) override;
    PosInItem getPosInItem(const QPoint &p, const QRectF &itemRect) override;
    DSchedule::Ptr getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate) override;
    bool IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond) override;
    bool JudgeIsCreate(const QPointF &pos) override;
    void RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos) override;
    QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime) override;
    QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime) override;

private:
    QVector<CScheduleItem *> m_vScheduleItem;
    QMargins m_margins; //四周空白
    bool m_LRFlag; //水平线
    QPen m_LRPen; //水平线画笔
    bool m_TBFlag; //垂直线
    QPen m_TBPen; //垂直线画笔
    QVector<int> m_vLRLarge; //大刻度像素位置
    qreal m_timeInterval;
    qreal m_sceneHeightScale = 0;

    QColor m_weekcolor = "#4F9BFF";
    QColor m_currenttimecolor = "#F74444";
    int m_currentTimeType = 0;
    QTimer *m_timer = nullptr;
    QMutex m_Mutex;
    bool m_updateDflag = false;
    DSchedule::List m_scheduleInfo;
    QDateTime m_currentDateTime;
    int m_minTime; //最小高度对应的最小时间
    int m_sMaxNum = 4;
};

#endif // GRAPHICSVIEW_H
