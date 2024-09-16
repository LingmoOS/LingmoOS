// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSyntaxHighlighter.h"
#include <QDebug>

CSyntaxHighlighter::CSyntaxHighlighter(QObject *parent):
    SyntaxHighlighter (parent),
    m_bHighlight(false)
{

}

CSyntaxHighlighter::CSyntaxHighlighter(QTextDocument *pDocument):
    SyntaxHighlighter (pDocument),m_bHighlight(false)
{

}

void CSyntaxHighlighter::setEnableHighlight(bool isEnable)
{
    m_bHighlight = isEnable;
}

void CSyntaxHighlighter::highlightBlock(const QString &text)
{
    if (!m_bHighlight) {
        return;
    }

    KSyntaxHighlighting::SyntaxHighlighter::highlightBlock(text);
}
