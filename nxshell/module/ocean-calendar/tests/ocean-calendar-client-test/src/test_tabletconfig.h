// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_TABLETCONFIG_H
#define TEST_TABLETCONFIG_H

#include "tabletconfig.h"
#include "gtest/gtest.h"

#include <QObject>

class test_TabletConfig : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    test_TabletConfig();

signals:

public slots:
};

#endif // TEST_TABLETCONFIG_H
