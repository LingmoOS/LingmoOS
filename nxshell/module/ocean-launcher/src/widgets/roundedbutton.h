// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROUNDEDBUTTON_H
#define ROUNDEDBUTTON_H

#include <QPushButton>

class RoundedButton : public QPushButton
{
    Q_OBJECT

public:
    explicit RoundedButton(QWidget *parent = nullptr);
    ~RoundedButton();

public slots:
    void setText(const QString &text);
};

#endif
