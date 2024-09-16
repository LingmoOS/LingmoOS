// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QDebug>
#include <QSize>
#include <QPixmap>
#include <QDir>

TEST(Utils_Destructor_UT, Utils_Destructor_UT)
{
    Utils *p = new Utils(nullptr);
    EXPECT_NE(p, nullptr);
    p->~Utils();
    p->deleteLater();
}

TEST(Utils_Constructor_UT, Utils_Constructor_UT)
{
    Utils *p = new Utils(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(Utils_getQssContent_UT, Utils_getQssContent_UT)
{
    QString result = Utils::getQssContent("");
    EXPECT_EQ(result.isEmpty(), true);
}

TEST(Utils_getConfigPath_UT, Utils_getConfigPath_UT)
{
    QString result = Utils::getConfigPath();
    EXPECT_EQ(result.isEmpty(), false);
}

TEST(Utils_isFontMimeType_UT, Utils_isFontMimeType_UT_001)
{
    bool result = Utils::isFontMimeType(QString(""));
    EXPECT_EQ(result, false);
}

TEST(Utils_suffixList_UT, Utils_suffixList_UT)
{
    QString result = Utils::suffixList();
    bool rs = result == QString("Font Files (*.ttf *.ttc *.otf)");
    EXPECT_EQ(rs, true);
}

TEST(Utils_renderSVG_UT, Utils_renderSVG_UT)
{
    QPixmap result = Utils::renderSVG(QString(""), QSize(20, 20));
}

TEST(Utils_loadFontFamilyFromFiles_UT, Utils_loadFontFamilyFromFiles_UT)
{
    QPixmap result = Utils::loadFontFamilyFromFiles(QString(""));
}

TEST(Utils_replaceEmptyByteArray_UT, Utils_replaceEmptyByteArray_UT)
{
    QByteArray source("12345\u0000\x01");
    QByteArray standardResult("12345");
    QByteArray result = Utils::replaceEmptyByteArray(source);
    bool compareResult = standardResult == result;
    EXPECT_EQ(compareResult, true);
}

class Utils_isErroCommand_UT_Param
{
public:
    Utils_isErroCommand_UT_Param(const QString &iCommand, Utils::CommandErrorType iType)
        : command(iCommand)
        , type(iType)
    {
    }
    QString command;
    Utils::CommandErrorType type;
};

class Utils_isErroCommand_UT : public ::testing::TestWithParam<Utils_isErroCommand_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(Utils, Utils_isErroCommand_UT, ::testing::Values(Utils_isErroCommand_UT_Param("权限", Utils::PermissionError), Utils_isErroCommand_UT_Param("permission", Utils::PermissionError), Utils_isErroCommand_UT_Param("请重试", Utils::RetryError), Utils_isErroCommand_UT_Param("retry", Utils::RetryError), Utils_isErroCommand_UT_Param("", Utils::NoError)));

bool stub_exists()
{
    return true;
}

QStringList stub_entryList(QDir::Filters filters, QDir::SortFlags sort)
{
    Q_UNUSED(filters);
    Q_UNUSED(sort);
    return QStringList() << "test"
                         << "test1"
                         << "test2"
                         << "test3"
                         << "test4"
                         << "test5";
}

bool stub_isFile()
{
    return true;
}

TEST_P(Utils_isErroCommand_UT, Utils_isErroCommand_UT_001)
{
    Utils_isErroCommand_UT_Param param = GetParam();
    Utils::CommandErrorType resultType = Utils::isErroCommand(param.command);
}

TEST(Utils_checkAndDeleteDir_UT, Utils_checkAndDeleteDir_UT)
{
    bool result = Utils::checkAndDeleteDir(QString("testdirstr__"));
    EXPECT_EQ(result, false);
}

TEST(Utils_deleteDir_UT, Utils_deleteDir_UT)
{
    Stub stub;
    stub.set((bool (QDir::*)() const)ADDR(QDir, exists), stub_exists);
    stub.set((QStringList(QDir::*)(QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryList), stub_entryList);
    stub.set(ADDR(QFileInfo, isFile), stub_isFile);
    bool result = Utils::deleteDir(QString("testdirstr__"));
    EXPECT_EQ(result, false);
}

TEST(Utils_replaceColorfulFont_UT, Utils_replaceColorfulFont_UT)
{
    QString source("12m");

    Utils::replaceColorfulFont(&source);
    EXPECT_EQ(source.isEmpty(), true);
}

TEST(Utils_isWayland_UT, Utils_isWayland_UT)
{
   bool res= Utils::isWayland();
   EXPECT_EQ(res, false)<<"check the status after isWayland()";
}
