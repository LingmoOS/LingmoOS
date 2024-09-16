// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULECOORMANAGE_H
#define SCHEDULECOORMANAGE_H

#include <QDate>
#include <QDateTime>
#include <QRect>

class CScheduleCoorManage
{
public:
    CScheduleCoorManage();
    ~CScheduleCoorManage();
    void setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin);
    void setDateRange(QDate begindate, QDate enddate);
    int getRightMagin() const
    {
        return m_rightmagin;
    }
    QRectF getDrawRegion(QDateTime begintime, QDateTime endtime);
    QRectF getDrawRegion(QDateTime begintime, QDateTime endtime, int index, int coount);
    QRectF getDrawRegion(QDate date, QDateTime begintime, QDateTime endtime, int index, int coount, int maxNum, int type = 0);
    QRectF getDrawRegionF(QDateTime begintime, QDateTime endtime);
    QRectF getAllDayDrawRegion(QDate begin, QDate end);
    QDateTime getDate(QPointF pos);
    QDate getsDate(QPointF pos);
    float getHeight(const QTime &time) const;
    QDate getBegindate() const
    {
        return m_begindate;
    }

private:
    int m_width {0};
    int m_height {0};
    QDate m_begindate;
    QDate m_enddate;
    qint64 m_totalDay {0};
    int m_rightmagin = 0;
};

#endif // SCHEDULECOORMANAGE_H
