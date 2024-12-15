// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADICONSWORKER_H
#define LOADICONSWORKER_H

#include "vntask.h"

#include <QObject>
#include <QRunnable>
//加载默认图标线程
class LoadIconsWorker : public VNTask
{
    Q_OBJECT
public:
    explicit LoadIconsWorker(QObject *parent = nullptr);
    //图标置灰
    QPixmap greyPix(QPixmap pix);

    const int DEFAULTICONS_COUNT = 10;

protected:
    //加载图标
    virtual void run();

signals:

public slots:
};

#endif // LOADICONSWORKER_H
