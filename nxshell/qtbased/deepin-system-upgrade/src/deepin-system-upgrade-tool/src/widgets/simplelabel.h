// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>

#include <QLabel>
#include <QColor>
#include <QPalette>

DWIDGET_USE_NAMESPACE

class SimpleLabel: public DLabel
{
    Q_OBJECT
public:
    explicit SimpleLabel(QWidget *parent=nullptr): DLabel(parent) {}
    void setColor(QColor color)
    {
        QPalette p = palette();
        p.setColor(foregroundRole(), color);
        setPalette(p);
    }
};
