/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "nodebuiltins_p.h"

using namespace KTextTemplate;

TextNode::TextNode(const QString &content, QObject *parent)
    : Node(parent)
    , m_content(content)
{
}

VariableNode::VariableNode(const FilterExpression &fe, QObject *parent)
    : Node(parent)
    , m_filterExpression(fe)
{
}

void VariableNode::render(OutputStream *stream, Context *c) const
{
    const auto v = m_filterExpression.resolve(c);
    if (!v.isValid())
        return;
    streamValueInContext(stream, v, c);
}

#include "moc_nodebuiltins_p.cpp"
