// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_FILELOADTHREAD_H
#define TEST_FILELOADTHREAD_H
#include "gtest/gtest.h"
#include <QObject>

class FileLoadThread;
class test_fileloadthread : public QObject
    , public ::testing::Test
{
public:
    test_fileloadthread();
    virtual void SetUp() override;
    virtual void TearDown() override;
    FileLoadThread *fthread;
};

#endif // TEST_FILELOADTHREAD_H
