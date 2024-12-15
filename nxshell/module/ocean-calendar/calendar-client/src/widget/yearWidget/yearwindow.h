// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef YEARWINDOW_H
#define YEARWINDOW_H

#include "animationstackedwidget.h"
#include "cschedulebasewidget.h"
#include "labelwidget.h"

#include <DFrame>
#include <DIconButton>
#include <DLabel>
#include <DWidget>

#include <QDate>
#include <QHBoxLayout>
#include <QGestureEvent>

DWIDGET_USE_NAMESPACE

class CYearView;
class CScheduleSearchView;
class YearFrame;
class CustomFrame;
class CYearScheduleOutView;

struct TouchGestureData {
    enum TouchMovingDirection {
        T_NONE,         //默认状态
        T_LEFT,         //往左
        T_TOP,          //往上
        T_RIGHT,        //往右
        T_BOTTOM        //往下
    }; //手势移动状态
    qreal length {0}; //手势移动距离
    qreal angle{0};         //手势移动角度
    TouchMovingDirection movingDirection{T_NONE};       // 手势移动方向
};

class CYearWindow: public CScheduleBaseWidget
{
    Q_OBJECT
public:
    explicit CYearWindow(QWidget *parent = nullptr);
    ~CYearWindow() override;
    //初始化ui界面
    void initUI();
    //初始化信号和槽的连接
    void initConnection();
    //设置系统主题
    void setTheMe(int type = 0);
    //设置是否在进行搜索
    void setSearchWFlag(bool flag);
    //设置选择时间年
    void setYearData() override;
    //更新显示时间
    void updateShowDate(const bool isUpdateBar = true) override;
    //更新日程显示
    void updateShowSchedule() override;
    //更新显示农历信息
    void updateShowLunar() override;
    //更新界面搜索日程显示
    void updateSearchScheduleInfo() override;
    //设置选中搜索日程
    void setSelectSearchScheduleInfo(const DSchedule::Ptr &info) override;
signals:
    /**
     * @brief signalsWUpdateShcedule
     */
    void signalsWUpdateShcedule();
    /**
     * @brief signalupdateschedule 更新日程的信号
     */
    void signalupdateschedule();
private slots:
    //切换上一年
    void slotprev();
    //切换到下一年
    void slotnext();
    //返回到当前时间
    void slottoday();
public slots:
    //隐藏日程浮框
    void slotSetScheduleHide();
    //接收鼠标点击
    void slotMousePress(const QDate &selectDate, const int pressType);
private:
    //切换年份
    void switchYear(const int offsetYear);
    //设置显示农历信息
    void setLunarShow();
protected:
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent *e) override;
private:
    //触摸手势处理
    bool gestureEvent(QGestureEvent *event);
    //轻切手势处理
    void tapGestureTriggered(QTapGesture *tap);
    //多指滑动手势处理
    void panTriggered(QPanGesture *pan);
    //计算方位角
    TouchGestureData calculateAzimuthAngle(QPointF &startPoint, QPointF &stopPoint);
private:
    DIconButton     *m_prevButton = nullptr;
    DIconButton     *m_nextButton = nullptr;
    LabelWidget          *m_today = nullptr;
    QDate           m_currentdate;
    QLabel          *m_yearLabel = nullptr;
    QLabel          *m_yearLunarLabel = nullptr;
    QLabel          *m_yearLunarDayLabel = nullptr;
    YearFrame       *m_yearWidget = nullptr;
    YearFrame       *m_firstYearWidget = nullptr;
    YearFrame       *m_secondYearWidget = nullptr;
    CustomFrame     *m_todayFrame = nullptr;
    AnimationStackedWidget  *m_StackedWidget = nullptr;
    QVBoxLayout *m_tMainLayout = nullptr;
    QString             m_searchText;
    bool m_searchFlag = false;
    DWidget             *m_topWidget = nullptr;
    //触摸开始坐标
    QPointF             m_touchBeginPoint;
    //触摸状态 0：初始状态 1:点击 2：移动
    int                 m_touchState{0};
    CYearScheduleOutView *m_scheduleView {nullptr};
};

class YearFrame : public QWidget
{
    Q_OBJECT
public:
    explicit YearFrame(DWidget *parent = nullptr);
    ~YearFrame() override;
    //设置显示时间
    void setShowDate(const QDate &selectDate);
    //设置阴历年显示
    void setLunarYearDate(const QString &lunar = "");
    //设置日期是否存在日程
    void setDateHasScheduleSign(const QSet<QDate> &hasSchedule);
    //设置不同主题颜色
    void setTheMe(int type = 0);
    //设置搜索日程
    void setSearchSchedule(const QSet<QDate> &hasSchedule);
    void setViewFocus(int index);
    int getViewFocusIndex();
private:
    //设置年信息显示
    void setYearShow();
signals:
    /**
     * @brief signalMousePress      鼠标点击事件触发信号
     * @param selectDate            选择时间
     * @param pressType             触发事件类型
     *          0:点击时间  1:双击时间  2: 双击月
     */
    void signalMousePress(const QDate &selectDate, const int pressType = 0);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QList<CYearView *>          m_monthViewList;
    QLabel                      *m_YearLabel = nullptr;
    QLabel                      *m_YearLunarLabel = nullptr;
    QDate                       m_selectDate;
    bool m_searchFlag = false;
    bool                        m_selectFlag = false;
    QString                     m_LunarYear;
    QString                     m_LunarDay;
    DWidget                     *m_topWidget = nullptr;
    int                         currentFocusView = -1;
};

#endif // YEARWINDOW_H
