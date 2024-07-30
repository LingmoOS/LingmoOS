/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef EXTENDSNODE_H
#define EXTENDSNODE_H

#include "node.h"
#include "template.h"

namespace KTextTemplate
{
class Parser;
}

class BlockNode;

using namespace KTextTemplate;

class ExtendsNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    explicit ExtendsNodeFactory(QObject *parent = {});

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class ExtendsNode : public Node
{
    Q_OBJECT
public:
    explicit ExtendsNode(const FilterExpression &fe, QObject *parent = {});
    ~ExtendsNode() override;

    void setNodeList(const NodeList &list);

    void render(OutputStream *stream, Context *c) const override;

    void appendNode(Node *node);

    Template getParent(Context *c) const;

    bool mustBeFirst() override
    {
        return true;
    }

private:
    FilterExpression m_filterExpression;
    NodeList m_list;
    QHash<QString, BlockNode *> m_blocks;
};

#endif
