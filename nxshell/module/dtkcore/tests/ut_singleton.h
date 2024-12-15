// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

#include "base/dsingleton.h"

class Singleton : public QObject
    , public Dtk::Core::DSingleton<Singleton>
{
    Q_OBJECT
    friend class Dtk::Core::DSingleton<Singleton>;

public:
    explicit Singleton(QObject *parent = nullptr);

    QAtomicInt count;
};

class MultiSingletonTester : public QObject
{
    Q_OBJECT
public:
    explicit MultiSingletonTester(QObject *parent = nullptr);

    int count() const;

public Q_SLOTS:
    void run();
};
