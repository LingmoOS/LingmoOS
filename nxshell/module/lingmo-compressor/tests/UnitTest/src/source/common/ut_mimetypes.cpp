// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypes.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/


/*******************************单元测试************************************/
TEST(testdetermineMimeType, TsetCase)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeSuffix(stub, "tar.bz2");
    determineMimeType("1.tar.bz2");

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_completeSuffix(stub1, "tar.lz4");
    determineMimeType("1.tar.lz4");

    Stub stub2;
    QFileInfoStub::stub_QFileInfo_completeSuffix(stub2, "tar.7z");
    determineMimeType("1.tar.7z");
}
