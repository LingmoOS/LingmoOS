// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_uncommentselection.h"
#include "../../src/common/settings.h"
#include "../../src/controls/tabbar.h"
#include "../../src/editor/editwrapper.h"
#include "../../src/widgets/window.h"
#include "../../src/startmanager.h"
#include "../../src/editor/dtextedit.h"
#include "../../src/editor/uncommentselection.h"

using namespace Comment;

test_uncommentselection::test_uncommentselection()
{
}

void test_uncommentselection::SetUp()
{
    comDef = new CommentDefinition;
}

void test_uncommentselection::TearDown()
{
    delete comDef;
}

TEST_F(test_uncommentselection, CommentDefinition)
{
    CommentDefinition num;
    ASSERT_FALSE(num.isAfterWhiteSpaces);
}

TEST_F(test_uncommentselection, setComments)
{
    QString singleLineComment = "a";
    QString multiLineCommentStart = "b";
    QString multiLineCommentEnd = "c";
    comDef->setComments(singleLineComment, multiLineCommentStart, multiLineCommentEnd);
    ASSERT_EQ(comDef->singleLine, singleLineComment);
    ASSERT_EQ(comDef->multiLineStart, multiLineCommentStart);
    ASSERT_EQ(comDef->multiLineEnd, multiLineCommentEnd);
}
//bool isValid() const;
TEST_F(test_uncommentselection, isValid)
{
    Comment::CommentDefinition numm;
    bool bRet = numm.isValid();

    ASSERT_TRUE(bRet == false);
}
//bool hasSingleLineStyle() const;
TEST_F(test_uncommentselection, hasSingleLineStyle)
{
    Comment::CommentDefinition numm;
    bool bRet = numm.hasSingleLineStyle();

    ASSERT_TRUE(bRet == false);
}
//bool hasMultiLineStyle() const;
TEST_F(test_uncommentselection, hasMultiLineStyle)
{
    Comment::CommentDefinition numm;
    bool bRet = numm.hasMultiLineStyle();

    ASSERT_TRUE(bRet == false);
}

