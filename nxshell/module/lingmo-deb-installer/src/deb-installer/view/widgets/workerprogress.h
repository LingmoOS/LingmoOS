// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKERPROGRESS_H
#define WORKERPROGRESS_H

#include "coloredprogressbar.h"

#include <DProgressBar>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class WorkerProgress : public DProgressBar
{
    Q_OBJECT

public:
    explicit WorkerProgress(QWidget *parent = nullptr);
};

#endif  // WORKERPROGRESS_H
