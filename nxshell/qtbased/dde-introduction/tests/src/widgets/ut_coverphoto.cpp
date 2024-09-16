// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coverphoto.h"

#include <gtest/gtest.h>

class ut_coverphoto_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    CoverPhoto *m_testModule = nullptr;
};

void ut_coverphoto_test::SetUp()
{
    m_testModule = new CoverPhoto();
}

void ut_coverphoto_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_coverphoto_test, setPixmap)
{
    ASSERT_TRUE(m_testModule);
    QPixmap pixmap;
    m_testModule->setPixmap(pixmap);
}

TEST_F(ut_coverphoto_test, paintEvent)
{
    QPaintEvent *event;
    m_testModule->paintEvent(event);
}
