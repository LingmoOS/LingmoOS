/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef NODE_BUILTINS_H
#define NODE_BUILTINS_H

#include "node.h"

namespace KTextTemplate
{

/**
  @internal

  A Node for plain text. Plain text is everything between variables, comments
  and template tags.
*/
class KTEXTTEMPLATE_EXPORT TextNode : public Node
{
    Q_OBJECT
public:
    explicit TextNode(const QString &content, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override
    { // krazy:exclude:inline
        Q_UNUSED(c);
        (*stream) << m_content;
    }

private:
    const QString m_content;
};

/**
  @internal

  A node for a variable or filter expression substitution.
*/
class KTEXTTEMPLATE_EXPORT VariableNode : public Node
{
    Q_OBJECT
public:
    explicit VariableNode(const FilterExpression &fe, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_filterExpression;
};
}

#endif
