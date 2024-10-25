/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: MIT
*/
#pragma once

#include <QObject>

#include <KTextEditor/Attribute>

namespace KTextEditor
{
class Editor;
}

class SemanticTokensLegend : public QObject
{
    enum TokenType {
        Unsupported = -1,
        Type,
        Class,
        Enum,
        Interface,
        Struct,
        TypeParameter,
        Parameter,
        Variable,
        Property,
        EnumMember,
        Event,
        Function,
        Method,
        Macro,
        Keyword,
        Modifier,
        Comment,
        String,
        Number,
        Regexp,
        Operator,
        Namespace
    };

public:
    explicit SemanticTokensLegend(QObject *parent = nullptr);

    /**
     * Called from LSP Server when capabilities are recieved
     */
    void initialize(const std::vector<QString> &types);

    KTextEditor::Attribute::Ptr attributeForTokenType(size_t idx) const
    {
        if (idx >= totalTokenTypes) {
            return {};
        }
        return sharedAttrs.at(idx);
    }

    size_t tokenTypeCount() const
    {
        return totalTokenTypes;
    }

private:
    Q_SLOT void themeChange(KTextEditor::Editor *e);
    void refresh(const std::vector<TokenType> &m_tokenTypes);

    size_t totalTokenTypes;
    std::vector<KTextEditor::Attribute::Ptr> sharedAttrs;
    KTextEditor::Attribute::Ptr fixedAttrs[7];
};
