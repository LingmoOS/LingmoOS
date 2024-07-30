/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef TEMPLATETAGNODE_H
#define TEMPLATETAGNODE_H

#include "node.h"

using namespace KTextTemplate;

class TemplateTagNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    TemplateTagNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class TemplateTagNode : public Node
{
    Q_OBJECT
public:
    /**
    The expression.
    Nodes to render if the expression is true
    Nodes to render if the expression is false
    */
    explicit TemplateTagNode(const QString &tagName, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

    static bool isKeyword(const QString &name);

private:
    QString m_name;
};

#endif
