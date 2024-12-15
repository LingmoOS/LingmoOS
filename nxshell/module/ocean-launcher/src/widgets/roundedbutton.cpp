// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roundedbutton.h"

RoundedButton::RoundedButton(QWidget *parent)
    : QPushButton(parent)
{
    setObjectName("RoundedButton");
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(60, 30);
}

RoundedButton::~RoundedButton()
{
}

void RoundedButton::setText(const QString &text)
{
    const QFontMetrics fm(fontMetrics());
    const int padding = 10;
    const QString elidedText = fm.elidedText(text, Qt::ElideRight, rect().width() - padding);

    QPushButton::setText(elidedText);
}
