// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOWINSTALLINFOTEXTEDIT_H
#define SHOWINSTALLINFOTEXTEDIT_H
#include <QTextEdit>
#include <QGestureEvent>
#include <QObject>
#include <QTimer>
#include <math.h>

#define CELL_TIME 15
#define TAP_MOVE_DELAY 300
// Tween算法(模拟惯性)
typedef std::function<void(qreal)> FunSlideInertial;
class FlashTween : public QObject
{
    Q_OBJECT
public:
    FlashTween();
    ~FlashTween() {}

public:
    void start(qreal t, qreal b, qreal c, qreal d, FunSlideInertial fSlideGesture);
    void stop() { m_timer->stop(); }
    bool active() { return m_timer->isActive(); }

private slots:
    void __run();

private:
    QTimer *m_timer = nullptr;
    FunSlideInertial m_fSlideGesture = nullptr;

    // 纵向单指惯性滑动
    qreal m_currentTime = 0;
    qreal m_beginValue = 0;
    qreal m_changeValue = 0;
    qreal m_durationTime = 0;
    qreal m_direction = 1;
    qreal m_lastValue = 0;

private:
    /**
    链接:https://www.cnblogs.com/cloudgamer/archive/2009/01/06/Tween.html
    效果说明
        Linear：无缓动效果；
        Quadratic：二次方的缓动（t^2）；
        Cubic：三次方的缓动（t^3）；
        Quartic：四次方的缓动（t^4）；
        Quintic：五次方的缓动（t^5）；
        Sinusoidal：正弦曲线的缓动（sin(t)）；
        Exponential：指数曲线的缓动（2^t）；
        Circular：圆形曲线的缓动（sqrt(1-t^2)）；
        Elastic：指数衰减的正弦曲线缓动；
        Back：超过范围的三次方缓动（(s+1)*t^3 - s*t^2）；
        Bounce：指数衰减的反弹缓动。
    每个效果都分三个缓动方式（方法），分别是：
        easeIn：从0开始加速的缓动；
        easeOut：减速到0的缓动；
        easeInOut：前半段从0开始加速，后半段减速到0的缓动。
        其中Linear是无缓动效果，没有以上效果。
    四个参数分别是：
        t: current time（当前时间）；
        b: beginning value（初始值）；
        c: change in value（变化量）；
        d: duration（持续时间）。
    */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
    static qreal quadraticEaseOut(qreal t, qreal b, qreal c, qreal d) { return -c * (t /= d) * (t - 2) + b; }

    static qreal cubicEaseOut(qreal t, qreal b, qreal c, qreal d) { return c * ((t = t / d - 1) * t * t + 1) + b; }

    static qreal quarticEaseOut(qreal t, qreal b, qreal c, qreal d) { return -c * ((t = t / d - 1) * t * t * t - 1) + b; }

    static qreal quinticEaseOut(qreal t, qreal b, qreal c, qreal d) { return c * ((t = t / d - 1) * t * t * t * t + 1) + b; }

    static qreal sinusoidalEaseOut(qreal t, qreal b, qreal c, qreal d) { return c * sin(t / d * (3.14 / 2)) + b; }

    static qreal circularEaseOut(qreal t, qreal b, qreal c, qreal d) { return c * sqrt(1 - (t = t / d - 1) * t) + b; }

    static qreal bounceEaseOut(qreal t, qreal b, qreal c, qreal d)
    {
        if ((t /= d) < (1 / 2.75)) {
            return c * (7.5625 * t * t) + b;
        } else if (t < (2 / 2.75)) {
            return c * (7.5625 * (t -= (1.5 / 2.75)) * t + .75) + b;
        } else if (t < (2.5 / 2.75)) {
            return c * (7.5625 * (t -= (2.25 / 2.75)) * t + .9375) + b;
        } else {
            return c * (7.5625 * (t -= (2.625 / 2.75)) * t + .984375) + b;
        }
    }
#pragma GCC diagnostic pop
};

/**
 * @brief The ShowInstallInfoTextEdit class
 * QTextEdit控件 修改部分触控事件
 */
class ShowInstallInfoTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ShowInstallInfoTextEdit(QWidget *parent = nullptr);

    // QObject interface
public:
    bool event(QEvent *event) override;

    // QWidget interface
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

    // QWidget interface
protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    /**
     * @brief gestureEvent 手势事件
     * @param event
     * @return
     */
    bool gestureEvent(QGestureEvent *event);

    /**
     * @brief tapGestureTriggered 单击手势事件
     */
    void tapGestureTriggered(QTapGesture *);

    /**
     * @brief tapAndHoldGestureTriggered 单指长按事件
     */
    void tapAndHoldGestureTriggered(QTapAndHoldGesture *);

    /**
     * @brief slideGesture 单指滑动手势(通过原生触摸屏事件进行抽象模拟)
     * @param diff
     * add for single refers to the sliding
     */
    void slideGesture(qreal diff);

    /**
     * @brief onSelectionArea 滑动选中事件
     */
    void onSelectionArea();

private:
    // 触摸屏手势动作
    enum GestureAction {
        GA_null,   // 无动作
        GA_tap,    // 点击
        GA_slide,  // 滑动
        GA_hold,   // 长按
    };

    GestureAction m_gestureAction = GA_null;  // 手势动作 默认误动作

    qint64 m_tapBeginTime = 0;  // 开始点击的时间

    bool m_slideContinue = false;  // 是否持续滑动

    // add for single refers to the sliding
    FlashTween tween;  // 滑动惯性

    qreal change = {0.0};    // 滑动变化量
    qreal duration = {0.0};  // 滑动方向

    // 鼠标事件的位置
    int m_start = 0;        // 开始时鼠标的位置
    int m_end = 0;          // 结束时鼠标的位置
    qreal m_stepSpeed = 0;  // 移动的速度

    int m_lastMousepos = 0;  // 上次移动后鼠标的位置

    ulong m_lastMouseTime = 0;  // 上次移动鼠标的时间

    int m_nSelectEndLine = 0;  //< 选择结束时后鼠标所在行
    int m_nSelectStart = 0;    //< 选择开始时的鼠标位置
    int m_nSelectEnd = 0;      //< 选择结束时的鼠标位置
};

#endif  // SHOWINSTALLINFOTEXTEDIT_H
