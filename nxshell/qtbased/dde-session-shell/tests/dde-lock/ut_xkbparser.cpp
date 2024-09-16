// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xkbparser.h"

#include <gtest/gtest.h>

class UT_XkbParser : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    XkbParser *m_parser;
};

void UT_XkbParser::SetUp()
{
    m_parser = new XkbParser;
}

void UT_XkbParser::TearDown()
{
    delete m_parser;
}

TEST_F(UT_XkbParser, basic)
{
    m_parser->lookUpKeyboardList(QStringList());
    m_parser->lookUpKeyboardKey(QString());
}
