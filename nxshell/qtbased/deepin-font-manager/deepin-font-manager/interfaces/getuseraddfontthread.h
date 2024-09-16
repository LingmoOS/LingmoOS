// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GETUSERADDFONTTHREAD_H
#define GETUSERADDFONTTHREAD_H

#include "dfontinfomanager.h"

#include <QThread>

class GetUserAddFontThread : public QThread
{
    Q_OBJECT

public:
    explicit GetUserAddFontThread(QObject *parent = nullptr);

protected:
    void run();
};

#endif // GETUSERADDFONTTHREAD_H
