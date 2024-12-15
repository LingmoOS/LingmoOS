// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DetectCode_H
#define UT_DetectCode_H
#include "gtest/gtest.h"
#include <QObject>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

class UT_DetectCode: public QObject, public::testing::Test
{
     Q_OBJECT
public:
    UT_DetectCode();
};


#endif
