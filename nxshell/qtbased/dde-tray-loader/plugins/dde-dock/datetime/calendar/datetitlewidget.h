// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DATETITLEWIDGET_H
#define DATETITLEWIDGET_H

#include <QWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class DateTitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DateTitleWidget(QWidget *parent = nullptr);

    void setDateLabelText(const QString &month, const int &day);

private:
    DLabel *m_dayofMonthLabel;
    DLabel *m_monthLabel;

};

#endif // DATETITLEWIDGET_H
