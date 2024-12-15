// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TOUCHGESTUREOPERATION_H
#define TOUCHGESTUREOPERATION_H

#include <QWidget>
class QGestureEvent;
class QTapGesture;
class QPanGesture;
class touchGestureOperation
{
public:
    /**
     * @brief The TouchMovingDirection enum  //手势移动方向
     */
    enum TouchMovingDirection {
        T_MOVE_NONE //默认状态
        ,
        T_LEFT //左
        ,
        T_TOP //上
        ,
        T_RIGHT //右
        ,
        T_BOTTOM //下
    };
    enum TouchState {
        T_STATE_NONE //默认状态
        ,
        T_SINGLE_CLICK //一指点击
        ,
        T_SLIDE //滑动手势

    };

private:
    enum MouseState {
        M_NONE //默认状态
        ,
        M_PRESS //点击
        ,
        M_MOVE //移动
    };

public:
    explicit touchGestureOperation(QWidget *parent = nullptr);
    /**
     * @brief event     事件处理
     * @param e         Qwidget事件
     * @return
     */
    bool event(QEvent *e);
    /**
     * @brief isUpdate  是否更新数据
     * @return
     */
    bool isUpdate() const;
    /**
     * @brief setUpdate     设置更新状态
     * @param b
     */
    void setUpdate(bool b);
    /**
     * @brief getTouchState 获取触摸手指状态
     * @return
     */
    TouchState getTouchState() const;
    /**
     * @brief getMovingDir  获取移动方向
     * @return
     */
    TouchMovingDirection getMovingDir() const;
    /**
     * @brief getTouchMovingDir             获取滑动方向
     * @param startPoint                    起始坐标
     * @param stopPoint                     结束坐标
     * @param movingLine                    移动距离
     * @return
     */
    static TouchMovingDirection getTouchMovingDir(QPointF &startPoint, QPointF &stopPoint, qreal &movingLine);

private:
    /**
     * @brief gestureEvent      触摸手势处理
     * @param event             手势事件
     * @return
     */
    bool gestureEvent(QGestureEvent *event);
    /**
     * @brief tapGestureTriggered       轻切手势处理
     * @param tap                       轻切手势事件
     */
    void tapGestureTriggered(QTapGesture *tap);
    /**
     * @brief panTriggered      多指滑动手势处理
     * @param pan               多指滑动手势
     */
    void panTriggered(QPanGesture *pan);
    /**
     * @brief calculateAzimuthAngle     计算方位角
     * @param startPoint                起始坐标
     * @param stopPoint                 结束坐标
     */
    void calculateAzimuthAngle(QPointF &startPoint, QPointF &stopPoint);

private:
    /**
     * @brief m_parentWidget        手势窗口
     */
    QWidget *m_parentWidget {nullptr};
    /**
     * @brief m_movingDir           移动方向
     */
    TouchMovingDirection m_movingDir {T_MOVE_NONE};
    /**
     * @brief m_beginTouchTime        触摸点击时间
     */
    qint64 m_beginTouchTime {0};
    /**
     * @brief m_beginTouchPoint        触摸点击坐标
     */
    QPointF m_beginTouchPoint {};
    /**
     * @brief m_touchState              触摸状态
     */
    TouchState m_touchState {T_STATE_NONE};
    /**
     * @brief m_touchState              鼠标的状态
     */
    MouseState m_mouseState {M_NONE};
    /**
     * @brief m_movelenght              移动距离
     */
    qreal m_movelenght {0};
    /**
     * @brief m_update                  更新标志
     */
    bool m_update {false};
};

#endif // TOUCHGESTUREOPERATION_H
