/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef BLOCKCONTEXT_H
#define BLOCKCONTEXT_H

#include <QHash>
#include <QMetaType>

class BlockNode;

class BlockContext
{
public:
    void addBlocks(const QHash<QString, BlockNode *> &blocks);

    BlockNode *pop(const QString &name);

    void push(const QString &name, BlockNode const *blockNode);

    BlockNode *getBlock(const QString &name) const;

    bool isEmpty() const;

    void remove(QList<BlockNode *> const &nodes);

private:
    QHash<QString, QList<BlockNode *>> m_blocks;
};

Q_DECLARE_METATYPE(BlockContext)

#endif
