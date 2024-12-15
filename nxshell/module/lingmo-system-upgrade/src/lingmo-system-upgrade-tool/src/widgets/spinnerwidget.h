// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkwidget_global.h>

#include <QWidget>

#include "simplelabel.h"

DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

class SpinnerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpinnerWidget(QWidget *parent = nullptr);
    void initUI();
    void setMessage(const QString &msg);
    void setMinimumSize(int width, int height);
    void setLabelSize(int size);
    void start();
    void stop();

private:
    SimpleLabel             *m_messageLabel;
    Dtk::Widget::DSpinner   *m_spinner;
};
