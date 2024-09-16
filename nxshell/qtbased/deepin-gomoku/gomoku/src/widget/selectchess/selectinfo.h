// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTINFO_H
#define SELECTINFO_H

#include <DLabel>

DWIDGET_USE_NAMESPACE
class Selectinfo : public DLabel
{
    Q_OBJECT
public:
    explicit Selectinfo(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // SELECTINFO_H
