// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_touchgestureoperation.h"

test_touchgestureoperation::test_touchgestureoperation()
{
    mTouchGestureOperation = new touchGestureOperation();
}

test_touchgestureoperation::~test_touchgestureoperation()
{
    delete mTouchGestureOperation;
    mTouchGestureOperation = nullptr;
}

//void touchGestureOperation::setUpdate(bool b)
TEST_F(test_touchgestureoperation, setUpdate)
{
    mTouchGestureOperation->setUpdate(true);
}

//bool touchGestureOperation::isUpdate() const
TEST_F(test_touchgestureoperation, isUpdate)
{
    mTouchGestureOperation->setUpdate(false);
    bool res = mTouchGestureOperation->isUpdate();
    EXPECT_FALSE(res);
    mTouchGestureOperation->setUpdate(true);
    res = mTouchGestureOperation->isUpdate();
    EXPECT_TRUE(res);
}

//touchGestureOperation::TouchState touchGestureOperation::getTouchState() const
TEST_F(test_touchgestureoperation, getTouchState)
{
    mTouchGestureOperation->getTouchState();
}

//touchGestureOperation::TouchMovingDirection touchGestureOperation::getMovingDir() const
TEST_F(test_touchgestureoperation, getMovingDir)
{
    mTouchGestureOperation->getMovingDir();
}

//touchGestureOperation::TouchMovingDirection touchGestureOperation::getTouchMovingDir(QPointF &startPoint, QPointF &stopPoint, qreal &movingLine)
TEST_F(test_touchgestureoperation, getTouchMovingDir)
{
    QPointF point1;
    QPointF point2;
    qreal num = 2;
    mTouchGestureOperation->getTouchMovingDir(point1, point2, num);
}

//void touchGestureOperation::calculateAzimuthAngle(QPointF &startPoint, QPointF &stopPoint)
TEST_F(test_touchgestureoperation, calculateAzimuthAngle)
{
    QPointF point1;
    QPointF point2;
    mTouchGestureOperation->calculateAzimuthAngle(point1, point2);
}
