// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/SyntaxHighlighter>

using namespace KSyntaxHighlighting;
class CSyntaxHighlighter : public SyntaxHighlighter
{
    Q_OBJECT
public:
    explicit CSyntaxHighlighter(QObject *parent = nullptr);
    explicit CSyntaxHighlighter(QTextDocument *pDocument);
    void setEnableHighlight(bool isEnable);

protected:
    virtual void highlightBlock(const QString & text) override;

private:
    bool m_bHighlight = false;
};
