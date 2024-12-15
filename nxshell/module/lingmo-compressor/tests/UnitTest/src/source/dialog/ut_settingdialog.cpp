// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QDir>
#include <QStandardPaths>

/*******************************函数打桩************************************/
QVariant dSettings_value_stub(const QString &key)
{
    return QVariant(true);
}
/*******************************单元测试************************************/
// 测试SettingDialog
class UT_SettingDialog : public ::testing::Test
{
public:
    UT_SettingDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SettingDialog;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SettingDialog *m_tester;
};

TEST_F(UT_SettingDialog, initTest)
{

}

TEST_F(UT_SettingDialog, test_getDefaultExtractPath)
{
    m_tester->m_curpath = "1/2";
    EXPECT_EQ(m_tester->getDefaultExtractPath(), "1/2");
}

TEST_F(UT_SettingDialog, test_isAutoCreatDir)
{
    Stub stub;
    stub.set(ADDR(DSettings, value), dSettings_value_stub);

    EXPECT_EQ(m_tester->isAutoCreatDir(), true);
}

TEST_F(UT_SettingDialog, test_isAutoOpen)
{
    Stub stub;
    stub.set(ADDR(DSettings, value), dSettings_value_stub);

    EXPECT_EQ(m_tester->isAutoOpen(), true);
}

TEST_F(UT_SettingDialog, test_isAutoDeleteFile)
{
    Stub stub;
    stub.set(ADDR(DSettings, value), dSettings_value_stub);

    EXPECT_EQ(m_tester->isAutoDeleteFile(), true);
}

TEST_F(UT_SettingDialog, test_isAutoDeleteArchive)
{
    m_tester->m_deleteArchiveOption->setValue("Never");
    EXPECT_EQ(m_tester->isAutoDeleteArchive(), "Never");
}

TEST_F(UT_SettingDialog, test_isAssociatedType)
{
    m_tester->isAssociatedType("application/zip");
}

TEST_F(UT_SettingDialog, test_slotClickCancelSelectAllButton)
{
    m_tester->slotClickCancelSelectAllButton();
    EXPECT_EQ(m_tester->m_settings->option("file_association.file_association_type.x-sv4crc")->value(), false);
}

TEST_F(UT_SettingDialog, testslotClickRecommendedButton)
{
    m_tester->slotClickRecommendedButton();
    EXPECT_EQ(m_tester->m_settings->option("file_association.file_association_type.x-iso9660-image")->value(), false);
    EXPECT_EQ(m_tester->m_settings->option("file_association.file_association_type.x-iso9660-appimage")->value(), false);
    EXPECT_EQ(m_tester->m_settings->option("file_association.file_association_type.x-source-rpm")->value(), false);
}

