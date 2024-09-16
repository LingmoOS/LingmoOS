// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMOOTHCURVEGENERATOR_H
#define SMOOTHCURVEGENERATOR_H

#include <QList>

class QPointF;
class QPainterPath;

class SmoothCurveGenerator
{
public:
    /**
     * Generate smooth curve by pass in point list.
     * @param points - point list
     * @return - return QPainterPath of smooth surve.
     */
    static QPainterPath generateSmoothCurve(const QList<QPointF> &points);

private:
    /**
     * Solves a tridiagonal system for one of coordinates (x or y)
     * of first Bezier control points.
     * @param result - Solution vector.
     * @param rhs - Right hand side vector.
     * @param n - Size of rhs.
     */
    static void calculateFirstControlPoints(double *&result, const double *rhs, int n);

    /**
     * Calculate control points of the smooth curve using the given knots.
     * @param knots - Points of the given curve.
     * @param firstControlPoints - Store the generated first control points.
     * @param secondControlPoints - Store the generated second control points.
     */
    static void calculateControlPoints(const QList<QPointF> &knots,
                                       QList<QPointF> *firstControlPoints,
                                       QList<QPointF> *secondControlPoints);
};
#endif // SMOOTHCURVEGENERATOR_H
