// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/chineseletterhelper.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QFile>

using namespace GrandSearch;
TEST(ChineseLetterHelperTest, ut_convertChinese2Pinyin)
{
    stub_ext::StubExt st;
    typedef bool (*fptr)(QFile *, QIODevice::OpenMode);
    fptr open_stub = (fptr)((bool (QFile::*)(QIODevice::OpenMode)) & QFile::open);
    st.set_lamda(open_stub, []() { return true; });
    st.set_lamda(&QFile::readAll, []() {
        return "0x6d4b:ce\n0x8bd5:shi";
    });

    QString firstPy;
    QString fullPy;
    Ch2PyIns->convertChinese2Pinyin("测试", firstPy, fullPy);
    EXPECT_EQ(firstPy, QString("cs"));
    EXPECT_EQ(fullPy, QString("ceshi"));
}
