/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "blockcontext.h"

#include "block.h"

void BlockContext::addBlocks(const QHash<QString, BlockNode *> &blocks)
{
    auto it = blocks.constBegin();
    const auto end = blocks.constEnd();

    for (; it != end; ++it) {
        m_blocks[it.key()].prepend(it.value());
    }
}

BlockNode *BlockContext::getBlock(const QString &name) const
{
    auto list = m_blocks[name];
    if (list.isEmpty())
        return nullptr;

    return list.last();
}

BlockNode *BlockContext::pop(const QString &name)
{
    QList<BlockNode *> &list = m_blocks[name];
    if (list.isEmpty())
        return nullptr;
    return list.takeLast();
}

void BlockContext::push(const QString &name, BlockNode const *blockNode)
{
    m_blocks[name].append(const_cast<BlockNode *>(blockNode));
}

bool BlockContext::isEmpty() const
{
    return m_blocks.isEmpty();
}

void BlockContext::remove(QList<BlockNode *> const &nodes)
{
    for (auto node : nodes) {
        m_blocks[node->name()].removeOne(node);
        if (m_blocks[node->name()].isEmpty()) {
            m_blocks.remove(node->name());
        }
    }
}
