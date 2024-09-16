// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#define protected public
#include "updatecontent.h"
#undef private
#undef protected

#include <QTest>

#include <gtest/gtest.h>

class TstUpdateContent : public testing::Test
{

};

TEST_F(TstUpdateContent, coverageTest)
{
    std::pair<QString, QString> pair;
    UpdateContent content(pair);
    Q_EMIT content.m_enterBtn->clicked();
    QResizeEvent event(QSize(100, 100), QSize(100, 200));
    content.resizeEvent(&event);
}
