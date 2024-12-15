// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include "scheduledatamanage.h"
#include "dschedule.h"

#include <QPainter>
#include <DWidget>
#include <QDateTime>
#include <QFont>

DWIDGET_USE_NAMESPACE

class ItemWidget : public DWidget
{
    Q_OBJECT
public:
    enum Item_Position { ItemTop,
                         ItemMiddle,
                         ItemBottom,
                         ItemOnly
    };

public:
    explicit ItemWidget(QWidget *parent = nullptr);

    void setPositon(const Item_Position &Positon);

    virtual void DrawItem(QPainter &painter);

    void setTitleContent(const QString &TitleContent);
    QString getTitleContent() const;

    QDate getShowDate() const;
    void setShowDate(const QDate &ShowDate);

    QDateTime getScheduleBeginTime() const;
    void setScheduleBeginTime(const QDateTime &ScheduleBeginTime);

    QDateTime getScheduleEndTime() const;
    void setScheduleEndTime(const QDateTime &ScheduleEndTime);

    DSchedule::Ptr scheduleInfo() const;
    void setScheduleInfo(const DSchedule::Ptr &scheduleInfo);

    DSchedule::Ptr getScheduleInfo() const;

protected:
    QFont getTitleFont() const;

    QColor getTitleColor() const;

    QFont getDateTimeFont() const;

    QColor getDateTimeColor() const;

    void setTitleColor(const QColor &TitleColor);

    void setDateTimeColor(const QColor &DateTimeColor);

    void setTitleFont(const QFont &TitleFont);

    void setDateTimeFont(const QFont &DateTimeFont);

protected:
    void paintEvent(QPaintEvent *event) override;
    void drawBackground(QPainter &painter);
    QColor getBackgroundColor();
    virtual void setTheMe(const int type);
    QColor ScheduleColor();
signals:

public slots:

private:
    Item_Position m_Positon {ItemMiddle};
    QString m_TitleContent;
    QDate m_ShowDate;
    QDateTime m_ScheduleBeginTime;
    QDateTime m_ScheduleEndTime;

    QFont m_TitleFont;
    QColor m_TitleColor;
    QFont m_DateTimeFont;
    QColor m_DateTimeColor;
    /**
     * @brief m_BackgroundColor  绘制背景色
     */
    QColor m_BackgroundColor{"#000000"};

    DSchedule::Ptr m_scheduleInfo;
};

#endif // ITEMWIDGET_H
