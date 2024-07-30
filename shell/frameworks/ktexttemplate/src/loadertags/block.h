/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef BLOCKNODE_H
#define BLOCKNODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class BlockNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    explicit BlockNodeFactory(QObject *parent = {});

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class BlockNode : public Node
{
    Q_OBJECT
    Q_PROPERTY(KTextTemplate::SafeString super READ getSuper)
public:
    explicit BlockNode(const QString &blockName, QObject *parent = {});
    ~BlockNode() override;

    void setNodeList(const NodeList &list) const;

    void render(OutputStream *stream, Context *c) const override;

    BlockNode *takeNodeParent();

    QString name() const;

    void setNodeParent(BlockNode *);

    void addParent(NodeList);

    NodeList nodeList() const;

    // Q_SLOTS
    /**
    Returns the block overridden by this one rendered in context.
    */
    SafeString getSuper() const;

private:
    const QString m_name;
    mutable NodeList m_list;
    mutable Context *m_context;
    mutable OutputStream *m_stream;
};

#endif
