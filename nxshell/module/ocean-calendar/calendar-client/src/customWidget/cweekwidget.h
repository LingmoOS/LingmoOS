// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CWEEKWIDGET_H
#define CWEEKWIDGET_H

#include <QPushButton>

class CWeekWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit CWeekWidget(QWidget *parent = nullptr);

    //设置一周首日
    void setFirstDay(Qt::DayOfWeek);
    //设置是否根据配置自动设置
    void setAutoFirstDay(bool);
    //设置字体大小是否跟随界面大小
    void setAutoFontSizeByWindow(bool);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_autoFirstDay = true; //是否根据配置自动设置
    bool m_autoFontSizeByWindow = true; //字体大小是否跟随界面大小

    Qt::DayOfWeek m_firstDay = Qt::Monday;  //一周首日
};

#endif // CWEEKWIDGET_H
