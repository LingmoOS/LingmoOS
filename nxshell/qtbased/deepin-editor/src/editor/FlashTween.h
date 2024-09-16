// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <math.h>
#include <functional>
#include <QObject>
#include <QTimer>
#define CELL_TIME   15
#define TAP_MOVE_DELAY 300


// Tween算法(模拟惯性)
typedef std::function<void (qreal)> FunSlideInertial;

class FlashTween :public QObject
{ 
    Q_OBJECT
public:
    FlashTween();
    ~FlashTween();

public:
    QTimer* m_timerY = nullptr;
    QTimer* m_timerX = nullptr;

    void startX(qreal t,qreal b,qreal c,qreal d, FunSlideInertial fSlideGesture);
    void startY(qreal t,qreal b,qreal c,qreal d, FunSlideInertial fSlideGesture);
    void stopX(){m_timerX->stop();}
    void stopY(){m_timerY->stop();}
    bool activeX(){return m_timerX->isActive();}
    bool activeY(){return m_timerY->isActive();}

private slots:
    void __runY();
    void __runX();

private:
    FunSlideInertial m_fSlideGestureX = nullptr;
    FunSlideInertial m_fSlideGestureY = nullptr;

    //纵向单指惯性滑动
    qreal m_currentTimeY = 0;
    qreal m_beginValueY = 0;
    qreal m_changeValueY = 0;
    qreal m_durationTimeY = 0;
    qreal m_directionY = 1;
    qreal m_lastValueY = 0;

    qreal m_currentTimeX = 0;
    qreal m_beginValueX = 0;
    qreal m_changeValueX = 0;
    qreal m_durationTimeX = 0;
    qreal m_directionX = 1;
    qreal m_lastValueX = 0;

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
    static qreal quadraticEaseOut(qreal t,qreal b,qreal c,qreal d){
        return -c *(t/=d)*(t-2) + b;
    }

    static qreal cubicEaseOut(qreal t,qreal b,qreal c,qreal d){
        return c*((t=t/d-1)*t*t + 1) + b;
    }

    static qreal quarticEaseOut(qreal t,qreal b,qreal c,qreal d){
        return -c * ((t=t/d-1)*t*t*t - 1) + b;
    }

    static qreal quinticEaseOut(qreal t,qreal b,qreal c,qreal d){
        return c*((t=t/d-1)*t*t*t*t + 1) + b;
    }

    static qreal sinusoidalEaseOut(qreal t,qreal b,qreal c,qreal d){
        return c * sin(t/d * (3.14/2)) + b;
    }

    static qreal circularEaseOut(qreal t,qreal b,qreal c,qreal d){
        return c * sqrt(1 - (t=t/d-1)*t) + b;
    }

    static qreal bounceEaseOut(qreal t,qreal b,qreal c,qreal d){
        if ((t/=d) < (1/2.75)) {
            return c*(7.5625*t*t) + b;
        } else if (t < (2/2.75)) {
            return c*(7.5625*(t-=(1.5/2.75))*t + .75) + b;
        } else if (t < (2.5/2.75)) {
            return c*(7.5625*(t-=(2.25/2.75))*t + .9375) + b;
        } else {
            return c*(7.5625*(t-=(2.625/2.75))*t + .984375) + b;
        }
    }
    #pragma GCC diagnostic pop
};
