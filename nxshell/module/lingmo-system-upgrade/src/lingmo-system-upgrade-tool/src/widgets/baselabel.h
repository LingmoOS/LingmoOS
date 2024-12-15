// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>
#include <DFontSizeManager>

#include <QWidget>
#include <QVBoxLayout>
#include <QColor>
#include <QFont>
#include <QString>

#include "simplelabel.h"

DWIDGET_USE_NAMESPACE

class BaseLabel: public QWidget
{
public:
    SimpleLabel *m_label;
    SimpleLabel *m_tip;

    explicit BaseLabel(QWidget *parent);
    void setText(QString text);
    void setTip(QString text);
    void setFont(QFont font);
    void setTextColor(QColor color);
    void setTipColor(QColor color);

private:
    QVBoxLayout *m_layout;

    void initUI();
};
