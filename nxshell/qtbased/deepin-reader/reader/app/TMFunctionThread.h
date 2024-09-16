// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TMFUNCTIONTHREAD_H
#define TMFUNCTIONTHREAD_H

#include <QThread>
#include <QVariant>

#include <functional>

typedef std::function<QVariant(void)> FNTemplate;
class TMFunctionThread : public QThread
{
    Q_OBJECT
public:
    explicit TMFunctionThread(QObject *parent = nullptr);
    ~TMFunctionThread();

public:
    FNTemplate func;
    QVariant result;

protected:
    void run() override;
};

#endif // TMFUNCTIONTHREAD_H
