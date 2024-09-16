// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXITLABEL_H
#define EXITLABEL_H

#include <QWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class ExitLabel : public DLabel
{
    Q_OBJECT
public:
    explicit ExitLabel(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // EXITLABEL_H
