// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEINTERVALWIDGET_H
#define TIMEINTERVALWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DLineEdit>
#include <DSettingsOption>

class TimeIntervalWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    TimeIntervalWidget();
    void setInterval(int time);
    int interval();
signals:
    void valueChanged(int v);
private:
    void initOption();

private Q_SLOTS:
    void onEditingFinished();

private:
    Dtk::Widget::DLabel *m_prefixTips = nullptr;
    Dtk::Widget::DLabel *m_suffixTips = nullptr;
    Dtk::Widget::DLineEdit *m_lineEdit = nullptr;
};

#endif   // TIMEINTERVALWIDGET_H
