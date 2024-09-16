// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DRAGINFOGRAPHICSVIEW_H
#define DRAGINFOGRAPHICSVIEW_H
#include "graphicsItem/draginfoitem.h"
#include "../widget/touchgestureoperation.h"
#include "dschedule.h"
#include "cgraphicsscene.h"

#include <DGraphicsView>

#include <QDate>
#include <QDrag>
#include <QColor>

DWIDGET_USE_NAMESPACE

class CScheduleDataManage;
class QPropertyAnimation;
class DragInfoGraphicsView : public DGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(int touchSlidePos READ getSlidePos WRITE setSlidePos)
public:
    explicit DragInfoGraphicsView(DWidget *parent = nullptr);
    ~DragInfoGraphicsView() override;
public:
//鼠标位置
    enum PosInItem {LEFT, MIDDLE, RIGHT, TOP, BOTTOM};
    //鼠标移动状态
    enum DragStatus {IsCreate = 0, ChangeBegin = 1, ChangeEnd = 2, ChangeWhole = 3, NONE = 4};
    /**
     * @brief The TouchState enum   触摸状态
     */
    enum TouchState {
        TS_NONE //默认状态
        , TS_PRESS //点击
        , TS_DRAG_MOVE //拖拽移动
        , TS_SLIDE //滑动
        , TS_LONG_PRESS //长按
    };

    int getDragStatus() const;
    void setShowRadius(bool leftShow = false, bool rightShow = false);
    //判断是否满足拖拽条件
    bool isCanDragge(const DSchedule::Ptr &info);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool event(QEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
private:
    void slotCreate();
    //设置点击选中日程
    void setPressSelectInfo(const DSchedule::Ptr &info);
    //拖拽更新日程信息
    void updateScheduleInfo(const DSchedule::Ptr &info);
    void DragPressEvent(const QPoint &pos, DragInfoItem *item);
    //鼠标左击释放数据处理
    void mouseReleaseScheduleUpdate();
    /**
     * @brief mousePress        鼠标左击事件处理
     * @param point             左击坐标
     */
    void mousePress(const QPoint &point);
    /**
     * @brief getSlidePos       获取滑动位置
     * @return
     */
    int getSlidePos() const;
    /**
     * @brief setSlidePos       设置滑动位置
     * @param pos
     */
    void setSlidePos(int pos);
    /**
     * @brief stopTouchAnimation    停止触摸滑动动画效果
     */
    void stopTouchAnimation();

protected:
    //删除日程
    void DeleteItem(const DSchedule::Ptr &info);

public:
    //设置搜索选中日程
    virtual void setSelectSearchSchedule(const DSchedule::Ptr &scheduleInfo);
    //初始化点击日程
    void pressScheduleInit();
    QDate getCurrentDate() const;
    void setCurrentDate(const QDate &currentDate);

protected:
    virtual void setDragPixmap(QDrag *drag, DragInfoItem *item);
    virtual void slotCreate(const QDateTime &date);
    //符合创建条件
    virtual bool MeetCreationConditions(const QDateTime &date) = 0;
    virtual void upDateInfoShow(const DragStatus &status = NONE, const DSchedule::Ptr &info = DSchedule::Ptr()) = 0;
    virtual QDateTime getPosDate(const QPoint &p) = 0;
    virtual void MoveInfoProcess(DSchedule::Ptr &info, const QPointF &pos) = 0;
    virtual PosInItem getPosInItem(const QPoint &p, const QRectF &itemRect) = 0;
    virtual DSchedule::Ptr getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate);
    virtual void ShowSchedule(DragInfoItem *infoitem);
    //设置主题
    virtual void setTheMe(int type = 0);
    //判断时间是否相等
    virtual bool IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond) = 0;
    //根据鼠标移动的距离判断是否创建日程
    virtual bool JudgeIsCreate(const QPointF &pos) = 0;
    virtual void RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos) = 0;
    /**
     * @brief getDragScheduleInfoBeginTime      获取拖拽日程开始时间
     * @param moveDateTime                      当前鼠标移动坐标对应的时间
     * @return
     */
    virtual QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime) = 0;
    /**
     * @brief getDragScheduleInfoEndTime    获取拖拽日程结束时间
     * @param moveDateTime                  当前鼠标移动坐标对应的时间
     * @return
     */
    virtual QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime) = 0;
    /**
     * @brief slideEvent            触摸滑动事件处理
     * @param startPoint            触摸开始坐标
     * @param stopPort              触摸结束坐标
     */
    virtual void slideEvent(QPointF &startPoint, QPointF &stopPort);
    //更新日程显示
    virtual void updateInfo();
    //更新背景上显示的item
    virtual void updateBackgroundShowItem() = 0;
signals:
    /**
     * @brief signalAngleDelta      发送滚动信号滚动相对量
     * @param delta     滚动相对量
     */
    void signalAngleDelta(int delta);
    //更新获取日程信息
    void signalsUpdateSchedule();
    /**
     * @brief signalScheduleShow        发送日程提示框信号
     * @param isShow                    是否显示
     * @param out                       显示的日程信息
     */
    void signalScheduleShow(const bool isShow, const DSchedule::Ptr &out = DSchedule::Ptr());
    //scene更新
    void signalSceneUpdate();
    void signalSwitchPrePage();
    void signalSwitchNextPage();
    void signalGotoDayView(const QDate &date);
public slots:
    //删除日程
    void slotDeleteItem();
    void slotSwitchPrePage(const QDate &focusDate, bool isSwitchView = false);
    void slotSwitchNextPage(const QDate &focusDate, bool isSwitchView = false);
    void slotContextMenu(CFocusItem *item);
    //切换焦点到下一个
    void slotsetNextFocus();

protected:
    //设置场景数据切换后当前item焦点
    virtual void setSceneCurrentItemFocus(const QDate &focusDate);

protected:
    int                                 m_themetype = 0;
    CGraphicsScene *m_Scene = nullptr;
    bool                                m_press = false;

    QAction *m_createAction = nullptr; // 创建日程
    QAction *m_editAction = nullptr;
    QAction *m_deleteAction = nullptr;
    DMenu *m_rightMenu = nullptr;

    QDateTime                           m_createDate;
    DragStatus                          m_DragStatus = NONE;
    bool                                m_isCreate;
    QDateTime                           m_PressDate;
    QDateTime                           m_MoveDate;
    QPoint                              m_PressPos;
    //保证月，周/日全天和非全天的拖拽日程为同一个
    static DSchedule::Ptr m_DragScheduleInfo;
    static bool m_hasUpdateMark; //拖拽后是否需要更新显示标志
    QDateTime                           m_InfoBeginTime;
    QDateTime                           m_InfoEndTime;
    QDrag *m_Drag = nullptr;
    //点击的原始info
    DSchedule::Ptr m_PressScheduleInfo;
    QRectF                              m_PressRect;
    /**
     * @brief m_TouchBeginPoint     触摸开始坐标
     */
    QPointF m_TouchBeginPoint;
    /**
     * @brief m_TouchBeginTime      触摸点击屏幕的事件
     */
    qint64 m_TouchBeginTime;
    /**
     * @brief m_touchState          触摸状态
     */
    TouchState m_touchState = TS_NONE;
    /**
     * @brief m_touchDragMoveState      触摸拖拽移动状态
     * 0 原始状态
     * 1 拖拽确认，移动的时候触发点击事件
     * 2 拖拽移动
     */
    int m_touchDragMoveState = 0;
    /**
     * @brief m_touchState          触摸滑动位置
     */
    int m_touchSlidePos {0};
    /**
     * @brief m_touchAnimation      触摸滑动动画
     */
    QPropertyAnimation *m_touchAnimation;
    /**
     * @brief m_touchMovingDir      记录快速滑动方向
     */
    touchGestureOperation::TouchMovingDirection m_touchMovingDir {touchGestureOperation::T_MOVE_NONE};
    qreal               m_radius{16};                   //圆角半径
    bool                m_leftShowRadius{false};        //左下角显示圆角
    bool                m_rightShowRadius{false};       //右下角显示圆角
    QColor              m_outerBorderColor;             //外边框背景色
    QDate m_currentDate;
};

#endif // DRAGINFOGRAPHICSVIEW_H
