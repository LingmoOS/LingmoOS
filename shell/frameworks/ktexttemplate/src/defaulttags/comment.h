/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef COMMENTNODE_H
#define COMMENTNODE_H

#include "node.h"

using namespace KTextTemplate;

class CommentNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    CommentNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class CommentNode : public Node
{
    Q_OBJECT
public:
    explicit CommentNode(QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;
};

#endif
