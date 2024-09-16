// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGSPINNERWIDGET_H
#define LOGSPINNERWIDGET_H

#include <DSpinner>
#include <DWidget>

class LogSpinnerWidget : public Dtk::Widget::DWidget
{
public:
    explicit LogSpinnerWidget(QWidget *parent = nullptr);
    void initUI();

    void spinnerStart();
    void spinnerStop();

private:
    Dtk::Widget::DSpinner *m_spinner{nullptr};
};

#endif  // LOGSPINNERWIDGET_H
