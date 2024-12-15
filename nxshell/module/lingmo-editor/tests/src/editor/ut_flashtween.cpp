// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_flashtween.h"

UT_FlashTween::UT_FlashTween()
{

}

//void startX(qreal t,qreal b,qreal c,qreal d, FunSlideInertial fSlideGesture);
TEST(UT_FlashTween_startX, UT_FlashTween_startX)
{
    FlashTween *a = new FlashTween();
    FunSlideInertial b;
    a->startX(1.1,1.1,1.1,1.1,b);
    int iRet = a->m_timerX->interval();
    ASSERT_TRUE(a->m_timerX->interval() == 15);

    a->deleteLater();
}

//void startY(qreal t,qreal b,qreal c,qreal d, FunSlideInertial fSlideGesture);
TEST(UT_FlashTween_startY, UT_FlashTween_startY)
{
    FlashTween *a = new FlashTween();
    FunSlideInertial b;
    a->startY(1.1,1.1,1.1,1.1,b);
    int iRet = a->m_timerY->interval();
    ASSERT_TRUE(a->m_timerY->interval() == 15);

    a->deleteLater();
}

//bool activeX(){return m_timerX->isActive();}
TEST(UT_FlashTween_activeX, UT_FlashTween_activeX)
{
    FlashTween *a = new FlashTween();
    FunSlideInertial b;
    a->m_timerX->start();
    bool bRet = a->activeX();
    ASSERT_TRUE(bRet == true);

    a->deleteLater();
}

//bool activeY(){return m_timerY->isActive();}
TEST(UT_FlashTween_activeY, UT_FlashTween_activeY)
{
    FlashTween *a = new FlashTween();
    FunSlideInertial b;
    a->m_timerY->start();
    bool bRet = a->activeY();
    ASSERT_TRUE(bRet == true);

    a->deleteLater();
}

TEST(UT_FlashTween_stopX, UT_FlashTween_stopX)
{
    FlashTween *a = new FlashTween();
    a->stopX();
    bool bRet = a->activeX();
    ASSERT_TRUE(bRet == false);

    a->deleteLater();
}

TEST(UT_FlashTween_stopY, UT_FlashTween_stopY)
{
    FlashTween *a = new FlashTween();
    a->stopY();
    bool bRet = a->activeY();
    ASSERT_TRUE(bRet == false);

    a->deleteLater();
}

TEST(UT_FlashTween___runY, UT_FlashTween___runY)
{
    FlashTween *a = new FlashTween();
    a->m_timerX = new QTimer;
    a->m_timerY = new QTimer;
    //a->__runY();

    EXPECT_NE(a->m_lastValueX,2.2);
    a->deleteLater();
    a->m_timerX->deleteLater();
    a->m_timerY->deleteLater();
}

TEST(UT_FlashTween___runX, UT_FlashTween___runX)
{
    FlashTween *a = new FlashTween();
    a->m_timerX = new QTimer;
    a->m_timerY = new QTimer;
    //a->__runX();

    EXPECT_NE(a->m_lastValueX,2.2);
    a->deleteLater();
    a->m_timerX->deleteLater();
    a->m_timerY->deleteLater();
}
