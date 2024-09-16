// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISPLAYCONTENT_TEST_H
#define DISPLAYCONTENT_TEST_H
#include <gtest/gtest.h>
#include <stub.h>
#include <QObject>
class displaycontent_test : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    displaycontent_test();

    // Test interface
protected:
    void SetUp();
    void TearDown();
};

#endif // DISPLAYCONTENT_TEST_H
