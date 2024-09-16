// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_CSYNTAXHIGHLIGHTER_H
#define TEST_CSYNTAXHIGHLIGHTER_H

#include "../../src/common/CSyntaxHighlighter.h"
#include "../../src/editor/dtextedit.h"

#include "gtest/gtest.h"
#include <QObject>

class test_CSyntaxHighlighter: public QObject, public::testing::Test
{
public:
    test_CSyntaxHighlighter();

    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter = nullptr;
};

#endif // TEST_CSYNTAXHIGHLIGHTER_H