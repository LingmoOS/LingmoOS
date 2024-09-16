// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "FlashTween.h"

FlashTween::FlashTween()
{
    m_timerX = new QTimer();
    QObject::connect(m_timerX, &QTimer::timeout, this ,&FlashTween::__runX);

    m_timerY = new QTimer();
    QObject::connect(m_timerY, &QTimer::timeout, this ,&FlashTween::__runY);
}

FlashTween::~FlashTween()
{
    if (m_timerX != nullptr) {
        delete m_timerX;
        m_timerX = nullptr;
    }

    if (m_timerY != nullptr) {
        delete m_timerY;
        m_timerY = nullptr;
    }
}

void FlashTween::startY(qreal t,qreal b,qreal c,qreal d, FunSlideInertial f)
{
    if(c==0.0 || d==0.0) return;
    m_currentTimeY = t;
    m_beginValueY = b;
    m_changeValueY = c;
    m_durationTimeY = d;

    m_lastValueY = 0;
    m_fSlideGestureY = f;
    m_directionY = m_changeValueY<0?1:-1;

    if (m_timerY != nullptr) {
    m_timerY->stop();
    m_timerY->start(CELL_TIME);
    }
}

void FlashTween::startX(qreal t,qreal b,qreal c,qreal d, FunSlideInertial f)
{
    if(c==0.0 || d==0.0) return;
    m_currentTimeX = t;
    m_beginValueX = b;
    m_changeValueX = c;
    m_durationTimeX = d;

    m_lastValueX = 0;
    m_fSlideGestureX = f;
    m_directionX = m_changeValueX<0?1:-1;

    m_timerX->stop();
    m_timerX->start(CELL_TIME);
}

void FlashTween::__runY()
{
    qreal tempValue = m_lastValueY;
    m_lastValueY = FlashTween::sinusoidalEaseOut(m_currentTimeY, m_beginValueY, abs(m_changeValueY), m_durationTimeY);
    m_fSlideGestureY(m_directionY*(m_lastValueY-tempValue));
    //qDebug()<<"###############################"<<m_lastValue<<temp<<m_lastValue-temp;

    if(m_currentTimeY<m_durationTimeY){
        m_currentTimeY+=CELL_TIME;
    }
    else {
        if (m_timerY != nullptr) {
        m_timerY->stop();
        }
    }
}

void FlashTween::__runX()
{
    qreal tempValue = m_lastValueX;
    m_lastValueX = FlashTween::sinusoidalEaseOut(m_currentTimeX, m_beginValueX, abs(m_changeValueX), m_durationTimeX);
    m_fSlideGestureX(m_directionX*(m_lastValueX - tempValue));
    //qDebug()<<"###############################"<<m_lastValue<<temp<<m_lastValue-temp;

    if(m_currentTimeX<m_durationTimeX){
        m_currentTimeX+=CELL_TIME;
    }
    else {
        m_timerX->stop();
    }
}

