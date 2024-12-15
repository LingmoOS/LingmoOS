// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulecoormanage.h"

#include <QTime>

CScheduleCoorManage::CScheduleCoorManage()
{
}

CScheduleCoorManage::~CScheduleCoorManage()
{
}

void CScheduleCoorManage::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    m_width = w;
    m_height = h;
    m_rightmagin = rightmagin;
    m_begindate = begindate;
    m_enddate = enddate;
    m_totalDay = begindate.daysTo(enddate) + 1;
}

void CScheduleCoorManage::setDateRange(QDate begindate, QDate enddate)
{
    m_begindate = begindate;
    m_enddate = enddate;
    m_totalDay = begindate.daysTo(enddate) + 1;
}

QRectF CScheduleCoorManage::getDrawRegion(QDateTime begintime, QDateTime endtime)
{
    QRectF rect;
    QString bb = begintime.toString("yyyyMMddhhmmsszzz");
    QString ee = endtime.toString("yyyyMMddhhmmsszzz");

    if (begintime > endtime)
        return rect;

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate)
        return rect;

    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay);
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay);
    qreal posY = m_height * (ScheduleBT / 86400.0);
    rect = QRectF(posX, posY, rWidth, rHeight);

    return rect;
}

QRectF CScheduleCoorManage::getDrawRegion(QDateTime begintime, QDateTime endtime, int index, int coount)
{
    QRectF rect;
    QString bb = begintime.toString("yyyyMMddhhmmsszzz");
    QString ee = endtime.toString("yyyyMMddhhmmsszzz");

    if (begintime > endtime)
        return rect;

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate)
        return rect;
    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay) / coount;
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
    qreal posY = m_height * (ScheduleBT / 86400.0);
    rect = QRectF(posX, posY, rWidth, rHeight);

    return rect;
}

QRectF CScheduleCoorManage::getDrawRegion(QDate date, QDateTime begintime, QDateTime endtime, int index, int coount, int maxNum, int type)
{
    QRectF rect;
    QString bb = begintime.toString("yyyyMMddhhmmsszzz");
    QString ee = endtime.toString("yyyyMMddhhmmsszzz");

    if (begintime > endtime)
        return rect;

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < date) {
        begindate = date;
        beginScheduleT = beginzero;
    }
    if (enddate > date) {
        enddate = date;
        endScheduleT = QTime(23, 59, 59);
    }

    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay) / coount;
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
    qreal posY = m_height * (ScheduleBT / 86400.0);

    if (coount > maxNum && type == 0) {
        qreal sscale = 27.0 / (m_width * (1.0 * day / m_totalDay));

        if (index < maxNum + 1) {
            rWidth = m_width * (1.0 * day / m_totalDay) * sscale + 0.5;
            posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
        } else {
            qreal trWidth = m_width * (1.0 * day / m_totalDay) * sscale + 0.5;
            rWidth = m_width * (1.0 * day / m_totalDay) - (index - 1) * trWidth;
            posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * trWidth;
        }
    }

    if (rHeight < 20) {
        if (posY + 20 > m_height)
            posY = m_height - 20;
        rHeight = 20;
    }

    if (posX < 1) {
        posX = 1;
        rWidth = rWidth - posX;
    }

    rect = QRectF(posX, posY, rWidth, rHeight);

    return rect;
}

QRectF CScheduleCoorManage::getDrawRegionF(QDateTime begintime, QDateTime endtime)
{
    QRectF rectf;

    if (begintime > endtime)
        return rectf;

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate)
        return rectf;

    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay);
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay);
    qreal posY = m_height * (ScheduleBT / 86400.0);
    rectf = QRectF(posX, posY, rWidth, rHeight);

    return rectf;
}

QRectF CScheduleCoorManage::getAllDayDrawRegion(QDate begin, QDate end)
{
    QRectF rect;
    if (begin > end)
        return rect;

    QDate begindate = begin;
    QDate enddate = end;

    if (begindate < m_begindate)
        begindate = m_begindate;
    if (enddate > m_enddate)
        enddate = m_enddate;

    qint64 beginday = m_begindate.daysTo(begindate);
    qint64 day = begindate.daysTo(enddate) + 1;
    qreal rWidth = m_width * (1.0 * day / m_totalDay) - 12;
    qreal rHeight = m_height;
    qreal posX = m_width * (1.0 * beginday / m_totalDay);
    qreal posY = 0;
    rect = QRectF(posX + 6, posY, rWidth - m_rightmagin, rHeight);

    return rect;
}

QDateTime CScheduleCoorManage::getDate(QPointF pos)
{
    QDateTime begintime;
    qint64 day = static_cast<qint64>((1.0 * pos.x() / m_width) * m_totalDay);

    if (day < 0) {
        day = 0;
    } else if (day >= m_totalDay) {
        day = m_totalDay - 1;
    }
    int time = static_cast<int>((1.0 * pos.y() / m_height) * 86400.0);
    int hours = time / 3600;
    int minutes = (time - 3600 * hours) / 60;
    int secss = time - 3600 * hours - 60 * minutes;
    QDate date = m_begindate.addDays(day);
    begintime.setDate(date);
    begintime.setTime(QTime(hours, minutes, secss));

    return begintime;
}

QDate CScheduleCoorManage::getsDate(QPointF pos)
{
    qint64 day = static_cast<qint64>((1.0 * pos.x() / m_width) * m_totalDay);

    if (day < 0) {
        day = 0;
    } else if (day >= m_totalDay) {
        day = m_totalDay - 1;
    }
    QDate date = m_begindate.addDays(day);

    return date;
}

float CScheduleCoorManage::getHeight(const QTime &time) const
{
    QTime beginzero(0, 0, 0);
    int ScheduleBT = beginzero.secsTo(time);
    float posY = static_cast<float>(m_height * (ScheduleBT / 86400.0));

    return posY;
}
