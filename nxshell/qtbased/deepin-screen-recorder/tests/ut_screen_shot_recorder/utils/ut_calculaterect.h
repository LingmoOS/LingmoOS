// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QPointF>
#include <gtest/gtest.h>
#include "../../src/utils/calculaterect.h"

using namespace testing;

class CalculaterectTest: public testing::Test
{

public:
    virtual void SetUp() override
    {
        qDebug() << "start CalculaterectTest";
    }

    virtual void TearDown() override
    {
        std::cout << "end CalculaterectTest" << std::endl;
    }
};

TEST_F(CalculaterectTest, test_getRotatePoint)
{
    QPointF point1(0, 0);
    QPointF point2(10, 10);
    QPointF point3(5, 5);
    QPointF point4(0, 5);
    qDebug() << getRotatePoint(point1, QPointF(5, 15), point3, QPointF(15, 5));
    qDebug() << getRotatePoint(point1, QPointF(5, 15), point3, QPointF(15, 25));
    qDebug() << getRotatePoint(point1, QPointF(25, 15), point3, QPointF(15, 35));
    qDebug() << getRotatePoint(point1, point2, point3, point4);
    //EXPECT_EQ(4, );

    QList<QPointF> points;
    points << point1 << point2 << point3 << point4;
    EXPECT_EQ(4, fourPointsOfLine(points).size());


    pointRotate(point1, point2, 1.0);
    FourPoints rectPoints;
    rectPoints << QPointF(0, 0) << QPointF(10, 0) << QPointF(0, 10) << QPointF(10, 10);
    getEightControlPoint(rectPoints);

    pointOfArrow(point1, point4, 5.0);
}

TEST_F(CalculaterectTest, test_base)
{
    QPointF point1(0, 0);
    QPointF point2(10, 10);
    QPointF point3(5, 5);
    QPointF point4(0, 5);
    QPointF pos(55, 55);
    QList<QPointF> points;
    FourPoints rectFPoints;
    FourPoints fourPoints;
    FourPoints mainPoints;
    rectFPoints << point1 << point2 << point3 << point4;
    fourPoints << point1 << point2 << point3 << point4;
    mainPoints << point1 << point2 << point3 << point4;

    EXPECT_FALSE(pointClickIn(QPointF(0, 0), QPointF(10, 10), 100));
    EXPECT_TRUE(pointOnLine(point1,  point2,  point3));
    EXPECT_EQ(5,  getDistance(point1, point4));

    EXPECT_DOUBLE_EQ(-0.54030230586813977, calculateAngle(point1,  point2,  point3));
    calculateAngle(QPointF(5, 5),  QPointF(5, 5),  point1);
    calculateAngle(QPointF(15, 25),  QPointF(10, 20),  QPointF(25, 15));
    calculateAngle(QPointF(15, 25),  QPointF(10, 20),  QPointF(5, 15));
    calculateAngle(QPointF(15, 25),  QPointF(10, 20),  QPointF(5, 45));

    /* get the new points after rotate with angle*/
    QPointF  pointRotate(QPointF point1, QPointF point2, qreal angle);

    /* the distance from a point(point3) to a line(point1, point2) */
    EXPECT_EQ(0, pointToLineDistance(point1,  point2,  point3));

    /* judge the direction of point3 of line(point1, point2) */
    EXPECT_EQ(-1,   pointLineDir(point1,  point2,  point3));

    /*get another four points in rectangle (Top/Bottom/Left/Right) */
    FourPoints getAnotherFPoints(FourPoints mainPoints);

    /* calculate the control point of the beizer */
    QPointF getControlPoint(QPointF point1, QPointF point2, bool direction) ;

    /* get eight control points */
    //QList<QPointF> getEightControlPoint(FourPoints rectFPoints);

    /* judge whether the clickOnPoint is on the bezier */
    /* 0 <= pos.x() <= 1*/
    EXPECT_FALSE(pointOnBezier(point1, point2,  point3, point4,  pos));

    /* judge whether the clickOnPoint is on the ellipse */
    EXPECT_FALSE(pointOnEllipse(rectFPoints, pos));

    QList<QPointF> pointsLine;
    pointsLine << point1 << point2 << point3 << point4;
    EXPECT_FALSE(pointOnArLine(pointsLine,  QPointF(55, 55)));

    //    /* resize arbitrary curved */
    EXPECT_EQ(2,  relativePosition(mainPoints, pos).size());
    FourPoints fourPointsvalue;
    fourPointsvalue << QPointF(8, 5) << QPointF(5, 6) << QPointF(9, 8) << QPointF(5, 1);
    QList<qreal> re;
    re << 55 << 7;
    QPointF po = getNewPosition(fourPointsvalue, re);
    qreal x = po.x();
    qreal y = po.y();
    EXPECT_DOUBLE_EQ(3.9017857142857135, x);
    EXPECT_DOUBLE_EQ(1.4553571428571428, y);
    re.clear();
    re << -2 << -2;
    getNewPosition(fourPointsvalue, re);

    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 7).size());


    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 7).size());

}
TEST_F(CalculaterectTest, test_point1Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    QPointF poss(125, 10);

    //    /***************** first point1 *******************/
    //    /* point1 in the first position */
    EXPECT_EQ(4,   point1Resize1(point1,  point2,  point3, point4,  pos).size());
    EXPECT_EQ(4,   point1Resize1(point1,  point2,  point3, point4,  pos, true).size());
    EXPECT_EQ(4,   point1Resize1(point1,  point2,  point3, point4,  poss, true).size());
    //    /* point1 in the second position*/
    EXPECT_EQ(4,   point1Resize2(QPointF(30, 20),  QPointF(40, 5),  QPointF(20, 15), QPointF(0, 5),  QPointF(55, 55)).size());
    EXPECT_EQ(4,   point1Resize2(QPointF(30, 20),  QPointF(40, 5),  QPointF(20, 15), QPointF(0, 5),  QPointF(55, 55), true).size());
    //    /* point1 in the third position */
    EXPECT_EQ(4,   point1Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(15, 0)).size());
    EXPECT_EQ(4,   point1Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(15, 0), true).size());

    EXPECT_EQ(4,   point1Resize3(QPointF(1737.03, 660.27), QPointF(1700, 856), QPointF(2095, 728), QPointF(2057.97, 923.73), QPointF(1736, 659), false).size());
    //    /* point1 in the fourth position */
    EXPECT_EQ(4,   point1Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 25), QPointF(20, 25),  QPointF(15, 20)).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(15, 100)).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(15, 100), true).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(325, 437),  QPointF(6.23, 313.69),  QPointF(219.69, 709.232), QPointF(20, 25),  QPointF(15, 100), true).size());
    //    /* point1 in the fifth position */
    EXPECT_EQ(4,   point1Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 5),  QPointF(110, 14)).size());
    EXPECT_EQ(4,   point1Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 5),  QPointF(110, 14), true).size());
    //    /* point1 in the sixth position */
    EXPECT_EQ(4,   point1Resize6(point1,  point2,  point3,  point4,  pos).size());
    EXPECT_EQ(4,   point1Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point1 in the seventh position */
    EXPECT_EQ(4,   point1Resize7(point1,  point2,  point3, point4,  pos).size());
    EXPECT_EQ(4,   point1Resize7(point1,  point2,  point3, point4,  pos, true).size());


    EXPECT_EQ(4,   point1Resize3(QPointF(1737.03, 660.27), QPointF(1700, 856), QPointF(2095, 728), QPointF(2057.97, 923.73), QPointF(1736, 659), false).size());
    EXPECT_EQ(4,   point1Resize3(QPointF(1736, 654), QPointF(1697.86, 855.594), QPointF(2096.11, 722.136), QPointF(2057.97, 923.73), QPointF(1735, 654), false).size());
    EXPECT_EQ(4,   point1Resize3(QPointF(1719, 911), QPointF(1625.49, 1110.27), QPointF(2026.91, 1055.49), QPointF(1933.4, 1254.76), QPointF(1720, 912), false).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(1077, 655), QPointF(853.931, 466.441), QPointF(919.867, 840.892), QPointF(696.798, 652.333), QPointF(1078, 655), false).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(1077, 655), QPointF(853.931, 466.441), QPointF(919.867, 840.892), QPointF(696.798, 652.333), QPointF(1078, 655), false).size());
    EXPECT_EQ(4,   point1Resize1(QPointF(310.249, 631.545), QPointF(520.033, 785.139), QPointF(506.967, 362.861), QPointF(716.751, 516.455), QPointF(310, 633), false).size());
    EXPECT_EQ(4,   point1Resize1(QPointF(310, 633), QPointF(519.252, 786.205), QPointF(507.498, 363.25), QPointF(716.751, 516.455), QPointF(309, 633), false).size());
    EXPECT_EQ(4,   point1Resize1(QPointF(309, 633), QPointF(518.903, 786.681), QPointF(506.847, 362.773), QPointF(716.751, 516.455), QPointF(308, 633), false).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(606, 703), QPointF(466.859, 461.599), QPointF(430.057, 804.411), QPointF(290.917, 563.01), QPointF(606, 704), false).size());
    EXPECT_EQ(4,   point1Resize2(QPointF(1135.91, 878.528), QPointF(1437.7, 607.095), QPointF(976.114, 700.863), QPointF(1277.9, 429.43), QPointF(1138, 877), false).size());
    EXPECT_EQ(4,   point1Resize2(QPointF(1138, 874), QPointF(1436.38, 605.633), QPointF(979.522, 697.798), QPointF(1277.9, 429.43), QPointF(1137, 873), false).size());
}
TEST_F(CalculaterectTest, test_point2Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** second point2 *******************/
    //    /* point2 in the first position */
    EXPECT_EQ(4,   point2Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(45, 100)).size());
    //    /* point2 in the second position */
    EXPECT_EQ(4,   point2Resize2(point1,  point2,  point3, point4,  pos).size());
    //    /* point2 in the third position */
    EXPECT_EQ(4,   point2Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(10, 40)).size());
    EXPECT_EQ(4,   point2Resize3(QPointF(103.35, 766.173), QPointF(54, 1027), QPointF(534.774, 847.802), QPointF(485.423, 1108.63), QPointF(54, 1026), false).size());
    //    /* point2 in the fourth position */
    EXPECT_EQ(4,   point2Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 125),  QPointF(15, 100)).size());
    //    /* point2 in the fifth position */
    EXPECT_EQ(4,   point2Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(25, 5),  QPointF(14, 140)).size());
    //    /* point2 in the sixth position */
    EXPECT_EQ(4,   point2Resize6(point1,  point2,  point3, point4,  pos).size());
    //    /* point2 in the seventh position */
    EXPECT_EQ(4,   point2Resize7(point1,  point2,  point3, point4,  pos).size());

    EXPECT_EQ(4,   point2Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(45, 100), true).size());
    //    /* point2 in the second position */
    EXPECT_EQ(4,   point2Resize2(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point2 in the third position */
    EXPECT_EQ(4,   point2Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(10, 40), true).size());
    //    /* point2 in the fourth position */
    EXPECT_EQ(4,   point2Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 125),  QPointF(15, 100), true).size());
    //    /* point2 in the fifth position */
    EXPECT_EQ(4,   point2Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(25, 5),  QPointF(14, 140), true).size());
    //    /* point2 in the sixth position */
    EXPECT_EQ(4,   point2Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point2 in the seventh position */
    EXPECT_EQ(4,   point2Resize7(point1,  point2,  point3, point4,  pos, true).size());


    EXPECT_EQ(4,   point2Resize3(QPointF(103.35, 766.173), QPointF(54, 1027), QPointF(534.774, 847.802), QPointF(485.423, 1108.63), QPointF(54, 1026), false).size());
    EXPECT_EQ(4,   point2Resize3(QPointF(104.133, 766.321), QPointF(55, 1026), QPointF(534.774, 847.802), QPointF(485.64, 1107.48), QPointF(54, 1027), false).size());
    EXPECT_EQ(4,   point2Resize3(QPointF(94.2441, 764.45), QPointF(44, 1030), QPointF(534.774, 847.802), QPointF(484.53, 1113.35), QPointF(44, 1031), false).size());
    EXPECT_EQ(4,   point2Resize2(QPointF(1222.74, 1198.04), QPointF(1415.95, 1010.65), QPointF(1099.5, 1070.99), QPointF(1292.71, 883.587), QPointF(1412, 1012), false).size());
    EXPECT_EQ(4,   point2Resize2(QPointF(1222.74, 1198.04), QPointF(1415.95, 1010.65), QPointF(1099.5, 1070.99), QPointF(1292.71, 883.587), QPointF(1412, 1012), false).size());
    EXPECT_EQ(4,   point2Resize2(QPointF(1221.5, 1196.77), QPointF(1412, 1012), QPointF(1099.5, 1070.99), QPointF(1290, 886.218), QPointF(1412, 1016), false).size());
    EXPECT_EQ(4,   point2Resize2(QPointF(1176.35, 1150.21), QPointF(1291, 1039), QPointF(1099.5, 1070.99), QPointF(1214.16, 959.777), QPointF(1295, 1036), false).size());
    EXPECT_EQ(4,   point2Resize4(QPointF(1622.08, 1158.04), QPointF(1567, 994), QPointF(1446.62, 1216.96), QPointF(1391.53, 1052.92), QPointF(1566, 993), false).size());
    EXPECT_EQ(4,   point2Resize4(QPointF(1635.31, 1153.6), QPointF(1574, 971), QPointF(1446.62, 1216.96), QPointF(1385.3, 1034.36), QPointF(1575, 971), false).size());
    EXPECT_EQ(4,   point2Resize1(QPointF(282, 641), QPointF(505.668, 804.759), QPointF(493.083, 352.696), QPointF(716.751, 516.455), QPointF(507, 808), false).size());
    EXPECT_EQ(4,   point2Resize1(QPointF(280.92, 642.475), QPointF(507, 808), QPointF(493.083, 352.696), QPointF(719.163, 518.221), QPointF(505, 810), false).size());

}
TEST_F(CalculaterectTest, test_point3Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** third point3 *******************/
    //    /* point3 in the first position */
    //

    // TTTTTTTTTTTTTTTTTTDDDDDDDDDDDDDDDDDDDDDDDDDDDd
    EXPECT_EQ(4,   point3Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(10, -14)).size());
    //    /* point3 in the second position */
    EXPECT_EQ(4,   point3Resize2(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(50, 40)).size());
    //    /* point3 in the third position */
    EXPECT_EQ(4,   point3Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 0)).size());
    EXPECT_EQ(4,   point3Resize3(QPointF(-31.7744, -38.4519), QPointF(-81.8158, 226.027), QPointF(404, 44), QPointF(353.959, 308.479), QPointF(403, 45), false).size());

    //    /* point3 in the fourth position */
    EXPECT_EQ(4,   point3Resize4(point1,  point2,  point3, point4,  pos).size());
    //    /* point3 in the fifth position */
    EXPECT_EQ(4,   point3Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 150),  QPointF(140, 140)).size());
    //    /* point3 in the sixth position */
    EXPECT_EQ(4,   point3Resize6(point1,  point2,  point3, point4,  pos).size());
    //    /* point3 in the seventh position */
    EXPECT_EQ(4,   point3Resize7(point1,  point2,  point3, point4,  pos).size());

    EXPECT_EQ(4,   point3Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(10, -14), true).size());
    //    /* point3 in the second position */
    EXPECT_EQ(4,   point3Resize2(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point3 in the third position */
    EXPECT_EQ(4,   point3Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 0), true).size());
    //    /* point3 in the fourth position */
    EXPECT_EQ(4,   point3Resize4(QPointF(30, 20),  QPointF(20, 10),  QPointF(25, 30), QPointF(0, 5),  QPointF(55, 55), true).size());
    //    /* point3 in the fifth position */
    EXPECT_EQ(4,   point3Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 150),  QPointF(140, 140), true).size());
    //    /* point3 in the sixth position */
    EXPECT_EQ(4,   point3Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point3 in the seventh position */
    EXPECT_EQ(4,   point3Resize7(point1,  point2,  point3, point4,  pos, true).size());

    EXPECT_EQ(4,   point3Resize3(QPointF(-31.5387, -39.6979), QPointF(-81.8158, 226.027), QPointF(408.857, 43.6283), QPointF(358.579, 309.353), QPointF(410, 44), false).size());
    EXPECT_EQ(4,   point3Resize3(QPointF(-31.567, -39.5479), QPointF(-81.8158, 226.027), QPointF(410, 44), QPointF(359.751, 309.575),  QPointF(409, 44), false).size());
    EXPECT_EQ(4,   point3Resize4(QPointF(1133, 656), QPointF(876.776, 439.415), QPointF(953.022, 868.918), QPointF(696.798, 652.333), QPointF(952, 869), false).size());
    EXPECT_EQ(4,   point3Resize4(QPointF(1132.44, 655.53), QPointF(876.776, 439.415), QPointF(952, 869), QPointF(696.331, 652.885), QPointF(951, 866), false).size());
    EXPECT_EQ(4,   point3Resize4(QPointF(1099.24, 627.461), QPointF(876.776, 439.415), QPointF(961, 791), QPointF(738.537, 602.954), QPointF(961, 792), false).size());
    EXPECT_EQ(4,   point3Resize2(QPointF(1243.53, 1063.76), QPointF(1372.51, 938.664), QPointF(1113.11, 929.297), QPointF(1242.09, 804.2), QPointF(1111, 932), false).size());
    EXPECT_EQ(4,   point3Resize2(QPointF(1241.09, 1066.13), QPointF(1372.51, 938.664), QPointF(1111, 932), QPointF(1242.41, 804.538), QPointF(1110, 932), false).size());
    EXPECT_EQ(4,   point3Resize2(QPointF(1241.09, 1066.13), QPointF(1372.51, 938.664), QPointF(1111, 932), QPointF(1242.41, 804.538), QPointF(1110, 932), false).size());
    EXPECT_EQ(4,   point3Resize1(QPointF(250.887, 646.54), QPointF(521.469, 844.647), QPointF(468, 350), QPointF(738.582, 548.108), QPointF(467, 349), false).size());
    EXPECT_EQ(4,   point3Resize1(QPointF(249.76, 645.714), QPointF(521.469, 844.647), QPointF(467, 349), QPointF(738.71, 547.933), QPointF(467, 350), false).size());
    EXPECT_EQ(4,   point3Resize2(QPointF(1129.67, 858.294), QPointF(1404.01, 611.555), QPointF(990, 703), QPointF(1264.34, 456.261), QPointF(989, 703), false).size());
}
TEST_F(CalculaterectTest, test_point4Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** fourth point4 *******************/
    //    /* point4 in the first position */
    EXPECT_EQ(4,   point4Resize1(point1,  point2, point3, point4, pos).size());
    //    /* point4 in the second position */
    EXPECT_EQ(4,   point4Resize2(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(20, 0)).size());
    //    /* point4 in the third position */
    EXPECT_EQ(4,   point4Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 40)).size());
    //    /* point4 in the fourth position */
    EXPECT_EQ(4,   point4Resize4(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 0)).size());
    //    /* point4 in the fifth position */
    EXPECT_EQ(4,   point4Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(140, 140)).size());
    //    /* point4 in the sixth position */
    EXPECT_EQ(4,   point4Resize6(point1,  point2, point3, point4, pos).size());
    //    /* point4 in the seventh position */
    EXPECT_EQ(4,   point4Resize7(point1,  point2, point3, point4, pos).size());

    EXPECT_EQ(4,   point4Resize1(QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 55), true).size());
    //    /* point4 in the second position */
    EXPECT_EQ(4,   point4Resize2(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(20, 0), true).size());
    //    /* point4 in the third position */
    EXPECT_EQ(4,   point4Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 40), true).size());
    //    /* point4 in the fourth position */
    EXPECT_EQ(4,   point4Resize4(point1,  point2, point3, point4, pos, true).size());
    //    /* point4 in the fifth position */
    EXPECT_EQ(4,   point4Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(140, 140), true).size());
    //    /* point4 in the sixth position */
    EXPECT_EQ(4,   point4Resize6(point1,  point2, point3, point4, pos, true).size());
    //    /* point4 in the seventh position */
    EXPECT_EQ(4,   point4Resize7(point1,  point2, point3, point4, pos, true).size());
    EXPECT_EQ(4,   point4Resize2(QPointF(1157.04, 1130.3), QPointF(1312, 980), QPointF(1075.98, 1046.73), QPointF(1230.94, 896.43), QPointF(1228, 896), false).size());
    EXPECT_EQ(4,   point4Resize2(QPointF(1157.04, 1130.3), QPointF(1315.23, 976.87), QPointF(1071.81, 1042.43), QPointF(1230, 889), QPointF(1232, 888), false).size());
    EXPECT_EQ(4,   point4Resize2(QPointF(1157.04, 1130.3), QPointF(1334.04, 958.619), QPointF(1073.99, 1044.68), QPointF(1251, 873), QPointF(1250, 874), false).size());
    EXPECT_EQ(4,   point4Resize3(QPointF(1304.12, 470.695), QPointF(1152.69, 576.628), QPointF(1398.7, 605.898), QPointF(1247.27, 711.83), QPointF(1244, 709), false).size());
    EXPECT_EQ(4,   point4Resize3(QPointF(1304.12, 470.695), QPointF(1145.32, 581.788), QPointF(1400.81, 608.907), QPointF(1242, 720), QPointF(1241, 722), false).size());
    EXPECT_EQ(4,   point4Resize1(QPointF(268.502, 659.436), QPointF(521.494, 844.665), QPointF(493.083, 352.696), QPointF(746.075, 537.925), QPointF(749, 541), false).size());
    EXPECT_EQ(4,   point4Resize1(QPointF(268.502, 659.436), QPointF(524.864, 847.133), QPointF(492.638, 353.304), QPointF(749, 541), QPointF(750, 541), false).size());
    EXPECT_EQ(4,   point4Resize1(QPointF(268.502, 659.436), QPointF(524.864, 847.133), QPointF(492.638, 353.304), QPointF(749, 541), QPointF(750, 541), false).size());
    EXPECT_EQ(4,   point4Resize4(QPointF(740.704, 644.331), QPointF(571.92, 363.939), QPointF(404.933, 846.451), QPointF(236.149, 566.058), QPointF(233, 567), false).size());
    EXPECT_EQ(4,   point4Resize4(QPointF(740.704, 644.331), QPointF(571.92, 363.939), QPointF(404.933, 846.451), QPointF(236.149, 566.058), QPointF(233, 567), false).size());
    EXPECT_EQ(4,   point4Resize4(QPointF(740.704, 644.331), QPointF(563.078, 349.249), QPointF(380.626, 861.082), QPointF(203, 566), QPointF(201, 566), false).size());
    EXPECT_EQ(4,   point4Resize4(QPointF(633.198, 654.966), QPointF(425.879, 426.485), QPointF(312.58, 945.888), QPointF(105.261, 717.407), QPointF(108, 721), false).size());
}
TEST_F(CalculaterectTest, test_point5Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /********************** fifth point5 ************************/
    //    /* point5 in the first position */
    EXPECT_EQ(4,   point5Resize1(point1,  point2,  point3, point4,  pos).size());
    //    /* point5 in the second position */
    EXPECT_EQ(4,   point5Resize2(QPointF(30, 20), QPointF(20, 10), QPointF(25, 30), QPointF(0, 5), QPointF(55, 55)).size());
    //    /* point5 in the third position */
    EXPECT_EQ(4,   point5Resize3(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -160)).size());
    //    /* point5 in the fourth position */
    EXPECT_EQ(4,   point5Resize4(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(30, -25)).size());
    //    /* point5 in the fifth position */
    EXPECT_EQ(4,   point5Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(60, 140)).size());
    //    /* point5 in the sixth position */
    EXPECT_EQ(4,   point5Resize6(point1,  point2,  point3, point4,  pos).size());
    //    /* point5 in the seventh position */
    EXPECT_EQ(4,   point5Resize7(point1,  point2,  point3, point4,  pos).size());

    EXPECT_EQ(4,   point5Resize1(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point5 in the second position */
    EXPECT_EQ(4,   point5Resize2(QPointF(30, 20), QPointF(40, 5), QPointF(20, 15), QPointF(0, 5), QPointF(55, 55), true).size());
    //    /* point5 in the third position */
    EXPECT_EQ(4,   point5Resize3(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -160), true).size());
    //    /* point5 in the fourth position */
    EXPECT_EQ(4,   point5Resize4(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(30, -25), true).size());
    //    /* point5 in the fifth position */
    EXPECT_EQ(4,   point5Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(60, 140), true).size());
    //    /* point5 in the sixth position */
    EXPECT_EQ(4,   point5Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point5 in the seventh position */
    EXPECT_EQ(4,   point5Resize7(point1,  point2,  point3, point4,  pos, true).size());

    EXPECT_EQ(4,    point5Resize3(QPointF(1068.5, 870.241), QPointF(1059.84, 877.019), QPointF(1101.16, 911.981), QPointF(1092.5, 918.759), QPointF(1065, 875), false).size());
    EXPECT_EQ(4,    point5Resize5(QPointF(892, 368), QPointF(892, 563), QPointF(1155, 368), QPointF(1155, 563), QPointF(891, 470), false).size());
    EXPECT_EQ(4,    point5Resize4(QPointF(1391.56, 1112.98), QPointF(1328.59, 925.457), QPointF(1254.62, 1158.96), QPointF(1191.65, 971.442), QPointF(1359, 1018), false).size());
    EXPECT_EQ(4,    point5Resize1(QPointF(292.128, 635.825), QPointF(511.669, 796.562), QPointF(497.21, 355.717), QPointF(716.751, 516.455), QPointF(401, 718), false).size());
}
TEST_F(CalculaterectTest, test_point6Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    /********************** sixth point6 ************************/
    /* point6 in the first position */
    EXPECT_EQ(4,    point6Resize1(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -25)).size());
    /* point6 in the second position */
    EXPECT_EQ(4,    point6Resize2(QPointF(30, 20), QPointF(40, 5), QPointF(20, 15), QPointF(0, 5), QPointF(55, 55)).size());
    /* point6 in the third position */
    EXPECT_EQ(4,    point6Resize3(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -50)).size());
    /* point6 in the fourth position */
    EXPECT_EQ(4,    point6Resize4(QPointF(30, 20), QPointF(20, 10), QPointF(25, 30), QPointF(0, 5), QPointF(55, 55)).size());
    /* point6 in the fifth position */
    EXPECT_EQ(4,    point6Resize5(point1, point2, point3, point4, pos).size());
    /* point6 in the sixth position */
    EXPECT_EQ(4,    point6Resize6(point1, point2, point3, point4, pos).size());
    /* point6 in the seventh position */
    EXPECT_EQ(4,    point6Resize7(point1, point2, point3, point4, pos).size());

    EXPECT_EQ(4,    point6Resize1(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -25), true).size());
    /* point6 in the second position */
    EXPECT_EQ(4,    point6Resize2(QPointF(30, 20), QPointF(40, 5), QPointF(20, 15), QPointF(0, 5), QPointF(55, 55), true).size());
    /* point6 in the third position */
    EXPECT_EQ(4,    point6Resize3(QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -50), true).size());
    /* point6 in the fourth position */
    EXPECT_EQ(4,    point6Resize4(QPointF(30, 20), QPointF(20, 10), QPointF(25, 30), QPointF(0, 5), QPointF(55, 55), true).size());
    /* point6 in the fifth position */
    EXPECT_EQ(4,    point6Resize5(point1, point2, point3, point4, pos, true).size());
    /* point6 in the sixth position */
    EXPECT_EQ(4,    point6Resize6(point1, point2, point3, point4, pos, true).size());
    /* point6 in the seventh position */
    EXPECT_EQ(4,    point6Resize7(point1, point2, point3, point4, pos, true).size());
    EXPECT_EQ(4,    point6Resize5(QPointF(-180, 836), QPointF(-180, 1307), QPointF(533, 836), QPointF(533, 1307), QPointF(172, 837), false).size());
    EXPECT_EQ(4,    point6Resize3(QPointF(1066.08, 867.147), QPointF(1057.42, 873.926), QPointF(1101.16, 911.981), QPointF(1092.5, 918.759), QPointF(1084, 892), false).size());
    EXPECT_EQ(4,    point6Resize1(QPointF(1129.76, 942.516), QPointF(1282.98, 977.382), QPointF(1151.06, 848.921), QPointF(1304.28, 883.786), QPointF(1141, 898), false).size());
}
TEST_F(CalculaterectTest, test_point7Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    /* point7 in the first position */
    EXPECT_EQ(4,    point7Resize1(point1, point2, point3, point4, pos).size());
    EXPECT_EQ(4,    point7Resize1(QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 15)).size());
    /* point7 in the second position */
    EXPECT_EQ(4,    point7Resize2(QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, -20), QPointF(0, -15)).size());
    /* point7 in the third position */
    EXPECT_EQ(4,    point7Resize3(QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, 20), QPointF(10, 15)).size());
    /* point7 in the fourth position */
    EXPECT_EQ(4,    point7Resize4(QPointF(30, 20),  QPointF(20, 10),  QPointF(25, 30), QPointF(0, 5),  QPointF(55, 55)).size());
    /* point7 in the fifth position */
    EXPECT_EQ(4,    point7Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140)).size());
    /* point7 in the sixth position */
    EXPECT_EQ(4,    point7Resize6(point1, point2, point3, point4, pos).size());
    /* point7 in the seventh position */
    EXPECT_EQ(4,    point7Resize7(point1, point2, point3, point4, pos, true).size());

    EXPECT_EQ(4,    point7Resize1(point1, point2, point3, point4, pos, true).size());
    /* point7 in the second position */
    EXPECT_EQ(4,    point7Resize2(QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, -20), QPointF(0, -15), true).size());
    /* point7 in the third position */
    EXPECT_EQ(4,    point7Resize3(QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, 20), QPointF(10, 15), true).size());
    /* point7 in the fourth position */
    EXPECT_EQ(4,    point7Resize4(QPointF(30, 20),  QPointF(20, 10),  QPointF(25, 30), QPointF(0, 5),  QPointF(55, 55), true).size());
    /* point7 in the fifth position */
    EXPECT_EQ(4,    point7Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140), true).size());
    /* point7 in the sixth position */
    EXPECT_EQ(4,    point7Resize6(point1, point2, point3, point4, pos, true).size());
    /* point7 in the seventh position */
    EXPECT_EQ(4,    point7Resize7(point1, point2, point3, point4, pos, true).size());

    EXPECT_EQ(4,    point7Resize4(QPointF(1131.19, 654.474), QPointF(876.776, 439.415), QPointF(960, 857), QPointF(705.581, 641.942), QPointF(831, 752), false).size());
    EXPECT_EQ(4,    point7Resize5(QPointF(928, 455), QPointF(928, 654), QPointF(1375, 455), QPointF(1375, 654), QPointF(1375, 557), false).size());
    EXPECT_EQ(4,    point7Resize2(QPointF(1222.74, 1198.04), QPointF(1380.07, 1045.45), QPointF(1081.67, 1052.6), QPointF(1239, 900), QPointF(1159, 975), false).size());
    EXPECT_EQ(4,    point7Resize3(QPointF(1314.12, 496.695), QPointF(1147.73, 613.094), QPointF(1413.39, 638.602), QPointF(1247, 755), QPointF(1329, 696), false).size());
}
TEST_F(CalculaterectTest, test_point8Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    /* point8 in the first position */
    EXPECT_EQ(4,    point8Resize1(QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 55)).size());
    /* point8 in the second position */
    EXPECT_EQ(4,    point8Resize2(point1, point2, point3, point4, pos).size());
    /* point8 in the third position */
    EXPECT_EQ(4,    point8Resize3(QPointF(30, 10), QPointF(20, 20), QPointF(35, 30), QPointF(0, 5), QPointF(10, 0)).size());
    /* point8 in the fourth position */
    EXPECT_EQ(4,    point8Resize4(QPointF(20, 30), QPointF(10, 10), QPointF(25, 5), QPointF(0, 5), QPointF(15, 5)).size());
    /* point8 in the fifth position */
    EXPECT_EQ(4,    point8Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140)).size());
    /* point8 in the sixth position */
    EXPECT_EQ(4,    point8Resize6(point1, point2, point3, point4, pos).size());
    /* point8 in the seventh position */
    EXPECT_EQ(4,    point8Resize7(point1, point2, point3, point4, pos).size());

    EXPECT_EQ(4,    point8Resize1(QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 55), true).size());
    /* point8 in the second position */
    EXPECT_EQ(4,    point8Resize2(point1, point2, point3, point4, pos, true).size());
    /* point8 in the third position */
    EXPECT_EQ(4,    point8Resize3(QPointF(30, 10), QPointF(20, 20), QPointF(35, 30), QPointF(0, 5), QPointF(10, 0), true).size());
    /* point8 in the fourth position */
    EXPECT_EQ(4,    point8Resize4(QPointF(20, 30), QPointF(10, 10), QPointF(25, 5), QPointF(0, 5), QPointF(15, 5), true).size());
    /* point8 in the fifth position */
    EXPECT_EQ(4,    point8Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140), true).size());
    /* point8 in the sixth position */
    EXPECT_EQ(4,    point8Resize6(point1, point2, point3, point4, pos, true).size());
    /* point8 in the seventh position */
    EXPECT_EQ(4,    point8Resize7(point1, point2, point3, point4, pos, true).size());

    EXPECT_EQ(4,    point8Resize5(QPointF(928, 455), QPointF(928, 639), QPointF(1365, 455), QPointF(1365, 639), QPointF(1142, 639), false).size());
    EXPECT_EQ(4,    point8Resize2(QPointF(1222.74, 1198.04), QPointF(1382.8, 1042.8), QPointF(1099.5, 1070.99), QPointF(1259.56, 915.745), QPointF(1316, 978), false).size());
    EXPECT_EQ(4,    point8Resize3(QPointF(792.639, 370.491), QPointF(703.349, 469.062), QPointF(1021.65, 577.938), QPointF(932.361, 676.509), QPointF(817, 573), false).size());
    EXPECT_EQ(4,    point8Resize4(QPointF(1429, 1100.4), QPointF(1366.3, 913.679), QPointF(1254.62, 1158.96), QPointF(1191.92, 972.235), QPointF(1277, 946), false).size());
    EXPECT_EQ(4,    point8Resize1(QPointF(268.502, 659.436), QPointF(496, 826), QPointF(493.083, 352.696), QPointF(720.581, 519.259), QPointF(608, 674), false).size());
}
TEST_F(CalculaterectTest, test_pointMoveMicro)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);

    FourPoints fourPoints;
    fourPoints << point1 << point2 << point3 << point4;

    //    /************************ micro-adjust  **************************/
    QString dir = "Left"; // Left Right Up Down
    bool isBig = true;
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   dir).size());
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   "Right").size());
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   "Up").size());
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   "Down").size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Left",  isBig).size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Right",  isBig).size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Up",  isBig).size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Down",  isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(20, 20) << QPointF(20, 5) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(20, 5) << QPointF(5, 5) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(5, 20) << QPointF(20, 15) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(20, 15) << QPointF(5, 15) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());

    fourPoints.clear();
    fourPoints << QPointF(30, 30) << QPointF(20, 15) << QPointF(5, 45) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
}

TEST_F(CalculaterectTest, test_getMainPoints)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    FourPoints fourPoints;
    fourPoints << point1 << point2 << point3 << point4;

    EXPECT_FALSE(pointInRect(fourPoints, pos));
    EXPECT_EQ(4,    getMainPoints(point1,  point2).size());
    EXPECT_EQ(4,    getMainPoints(point1,  point2, true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(30, 40), true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(30, 20), true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(10, 40), true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(10, 20), true).size());

}
