// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OPENCALENDARWIDGET_H
#define OPENCALENDARWIDGET_H

#include <DWidget>
#include <QColor>
#include <QFont>

DWIDGET_USE_NAMESPACE
class OpenCalendarWidget : public DWidget
{
    Q_OBJECT
public:
    explicit OpenCalendarWidget(QWidget *parent = nullptr);

    void setScheduleCount(int ScheduleCount);

    QFont TitleFont();
    QColor TitleColor() const;
    void setTitleColor(const QColor &TitleColor);

private:
    void setTheMe(const int type);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
signals:

public slots:
private:
    int m_ScheduleCount;
    QColor m_TitleColor;
    QFont m_TitleFont;
};

#endif // OPENCALENDARWIDGET_H
