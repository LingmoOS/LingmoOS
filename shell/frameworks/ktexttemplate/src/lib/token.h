/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_TOKEN_H
#define KTEXTTEMPLATE_TOKEN_H

#include <QString>

namespace KTextTemplate
{

/**
  @internal

  The available token types.
*/
enum TokenType {
    TextToken, ///< The Token is a text fragment
    VariableToken, ///< The Token is a variable node
    BlockToken, ///< The Token is a block, ie, part of a tag
    CommentToken ///< The Token is a comment node.
};

/// @headerfile token.h <KTextTemplate/Token>

/**
  A token in a parse stream for a template.

  This class is only relevant for template tag implementations.
*/
struct Token {
    int tokenType; ///< The Type of this Token
    int linenumber; ///< The line number this Token starts at
    QString content; ///< The content of this Token
};
}

Q_DECLARE_TYPEINFO(KTextTemplate::Token, Q_RELOCATABLE_TYPE);

#endif
