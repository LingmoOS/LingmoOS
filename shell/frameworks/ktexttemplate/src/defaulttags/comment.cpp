/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "comment.h"

#include "parser.h"

CommentNodeFactory::CommentNodeFactory() = default;

Node *CommentNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    Q_UNUSED(tagContent);

    p->skipPast(QStringLiteral("endcomment"));

    return new CommentNode(p);
}

CommentNode::CommentNode(QObject *parent)
    : Node(parent)
{
}

void CommentNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(stream);
    Q_UNUSED(c);
}

#include "moc_comment.cpp"
