// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "ui_common.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>




/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

TEST(UT_UICOMMON, formatColor)
{
    QColor color(10, 10, 10);
    QString colorStr = formatColor(QColor(10, 10, 10), 10);

    EXPECT_EQ(colorStr, QString("A:%1 R:%2 G:%3 B:%4")
              .arg(color.alpha())
              .arg(color.red())
              .arg(color.green())
              .arg(color.blue()));
}

TEST(UT_UICOMMON, iconPathFromQrc)
{
    QString iconPath = iconPathFromQrc(QString("11"));

    EXPECT_EQ(iconPath, QString(":/icons/deepin/builtin/11"));
}
