// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"

#include <QWidget>

class QPropertyAnimation;

#define VALUE_CHANGE_INTERVAL_MS 20

DEF_NAMESPACE_BEGIN
DEF_ANTIVIRUS_NAMESPACE_BEGIN

class ScoreProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentValue READ currentValue WRITE setCurrentValue USER true)
public:
    explicit ScoreProgressBar(QWidget *parent = nullptr);
    ~ScoreProgressBar();

    QSize minimumSizeHint() const;
    // 设置当前显示值
    void setCurrentValue(int value);
    // 获取当前显示值
    int currentValue() const;
    void setValue(int value);

    inline void setColor(QColor color) { m_color = color; }

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);

private:
    // 当前显示值
    int m_currentValue;
    // 设定值
    int m_value;
    // 进度条颜色
    QColor m_color;
    // 进度条数值动画
    QPropertyAnimation *m_valueAnimation;
};

DEF_ANTIVIRUS_NAMESPACE_END
DEF_NAMESPACE_END
