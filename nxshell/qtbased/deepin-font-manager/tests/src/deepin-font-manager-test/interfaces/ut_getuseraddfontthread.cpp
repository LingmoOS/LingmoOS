// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "getuseraddfontthread.h"
#include "dcomworker.h"
#include "dfontpreviewlistdatathread.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
//#include "commonheaderfile.h"


#include <QSignalSpy>

namespace {
class TestGetUserAddFontThread : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new GetUserAddFontThread();
    }
    void TearDown()
    {
        delete fm;
    }

    GetUserAddFontThread *fm;
};
}

TEST_F(TestGetUserAddFontThread, initTest)
{

}

void getChineseAndMonoFont_stub()
{
    return;
}
void onRefreshUserAddFont_stub(QList<DFontInfo> &fontInfoList)
{
    return;
}

QStringList getAllFclistPathList_stub()
{
    QStringList strlist;
    strlist << "111" << "22334";
    return strlist;
}

TEST_F(TestGetUserAddFontThread, checkrun)
{
    Stub stub;
    stub.set(ADDR(FontManager, getChineseAndMonoFont), getChineseAndMonoFont_stub);
    stub.set(ADDR(DFontPreviewListDataThread, onRefreshUserAddFont), onRefreshUserAddFont_stub);
    stub.set(ADDR(DFontInfoManager, getAllFclistPathList), getAllFclistPathList_stub);
    fm->run();
}
