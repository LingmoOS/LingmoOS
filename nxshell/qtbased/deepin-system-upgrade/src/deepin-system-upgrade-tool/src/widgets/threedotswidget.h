// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QColor>
#include <QWidget>

class ThreeDotsWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ThreeDotsWidget(QWidget *parent=nullptr, int aDot = 0);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int     m_activeDot = 0;
    QColor  m_activeColor;
    QColor  m_deactiveColor;

    void initUI();
};
