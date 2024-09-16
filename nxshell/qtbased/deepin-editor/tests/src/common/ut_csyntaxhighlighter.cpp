// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_csyntaxhighlighter.h"

test_CSyntaxHighlighter::test_CSyntaxHighlighter()
{

}

TEST_F(test_CSyntaxHighlighter, setEnableHighlight)
{

    TextEdit *m_pTextEdit = new TextEdit();
    bool bRet = true;
    CSyntaxHighlighter *pCSyntaxHighlighter = new CSyntaxHighlighter(m_pTextEdit->document());
    pCSyntaxHighlighter->setEnableHighlight(bRet);

    EXPECT_EQ(pCSyntaxHighlighter->m_bHighlight,true);

    EXPECT_NE(m_pTextEdit,nullptr);
    EXPECT_NE(pCSyntaxHighlighter,nullptr);

    pCSyntaxHighlighter->deleteLater();
    m_pTextEdit->deleteLater();


    
}

TEST_F(test_CSyntaxHighlighter, setEnableHighlight2)
{

    TextEdit *m_pTextEdit = new TextEdit();
    bool bRet = true;
    CSyntaxHighlighter *pCSyntaxHighlighter = new CSyntaxHighlighter(this);
    pCSyntaxHighlighter->setEnableHighlight(bRet);

    EXPECT_EQ(pCSyntaxHighlighter->m_bHighlight,true);

    EXPECT_NE(m_pTextEdit,nullptr);
    EXPECT_NE(pCSyntaxHighlighter,nullptr);

    pCSyntaxHighlighter->deleteLater();
    m_pTextEdit->deleteLater();

    
}

TEST_F(test_CSyntaxHighlighter, highlightBlock)
{

    TextEdit *m_pTextEdit = new TextEdit();
    bool bRet = true;
    const QString strTest = "12345";
    CSyntaxHighlighter *pCSyntaxHighlighter = new CSyntaxHighlighter(m_pTextEdit->document());
    pCSyntaxHighlighter->m_bHighlight = false;
    pCSyntaxHighlighter->highlightBlock(strTest);

    EXPECT_EQ(pCSyntaxHighlighter->m_bHighlight,false);
    EXPECT_NE(m_pTextEdit,nullptr);
    EXPECT_NE(pCSyntaxHighlighter,nullptr);

    pCSyntaxHighlighter->deleteLater();
    m_pTextEdit->deleteLater();

    
}

TEST_F(test_CSyntaxHighlighter, highlightBlock1)
{

    TextEdit *m_pTextEdit = new TextEdit();
    bool bRet = true;
    const QString strTest = "12345";
    CSyntaxHighlighter *pCSyntaxHighlighter = new CSyntaxHighlighter(m_pTextEdit->document());
    pCSyntaxHighlighter->m_bHighlight = true;
    pCSyntaxHighlighter->highlightBlock(strTest);

    EXPECT_EQ(pCSyntaxHighlighter->m_bHighlight,true);
    EXPECT_NE(m_pTextEdit,nullptr);
    EXPECT_NE(pCSyntaxHighlighter,nullptr);

    pCSyntaxHighlighter->deleteLater();
    m_pTextEdit->deleteLater();


    
}

