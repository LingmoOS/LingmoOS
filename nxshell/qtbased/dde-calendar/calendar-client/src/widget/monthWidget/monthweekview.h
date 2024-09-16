// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHWEEKVIEW_H
#define MONTHWEEKVIEW_H

#include "constants.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE
class WeekRect;
/**
 * @brief The CMonthWeekView class
 *      月视图周显示
 */
class CMonthWeekView : public DWidget
{
    Q_OBJECT
public:
    explicit CMonthWeekView(QWidget *parent = nullptr);
    ~CMonthWeekView() override;
    void setFirstDay(const Qt::DayOfWeek weekday);
    void setTheMe(int type = 0);
    void updateWeek();
    //设置当前时间
    void setCurrentDate(const QDate &currentDate);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    QColor m_backgroundColor;
    QVector<WeekRect *>         m_weekRect;
    int                         m_radius = 8;
    Qt::DayOfWeek               m_firstWeek{Qt::Sunday};
    Qt::DayOfWeek               m_currentWeek{Qt::Monday};
};


class WeekRect
{
public:
    WeekRect();
    void setWeek(const Qt::DayOfWeek &showWeek, const bool &showLine = false);
    void setRect(const QRectF &rectF);
    void paintRect(QPainter &painter);
    void setTheMe(int type = 0);
private:
    QColor          m_activeColor;
    QColor          m_testColor;
    Qt::DayOfWeek   m_showWeek;
    QString         m_weekStr;
    QFont           m_font;
    QRectF          m_rectF;
    bool            m_showLine;
    qreal           m_lineHeight{2};
};

#endif // WEEKINDICATOR_H
