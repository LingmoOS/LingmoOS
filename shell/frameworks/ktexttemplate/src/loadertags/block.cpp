/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "block.h"

#include "blockcontext.h"
#include "exception.h"
#include "parser.h"
#include "rendercontext.h"
#include "util.h"

static const char *const __loadedBlocks = "__loadedBlocks";

// Terrible hack warning.
#define BLOCK_CONTEXT_KEY nullptr

BlockNodeFactory::BlockNodeFactory(QObject *parent)
    : AbstractNodeFactory(parent)
{
}

Node *BlockNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    const auto expr = tagContent.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    if (expr.size() != 2) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("block tag takes one argument"));
    }

    const auto blockName = expr.at(1);

    auto loadedBlocksVariant = p->property(__loadedBlocks);
    QVariantList blockVariantList;

    if (loadedBlocksVariant.isValid() && loadedBlocksVariant.userType() == qMetaTypeId<QVariantList>()) {
        blockVariantList = loadedBlocksVariant.value<QVariantList>();
        for (auto &item : blockVariantList) {
            const auto blockNodeName = item.value<QString>();

            if (blockNodeName == blockName) {
                throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("'block' tag with name '%1' appears more than once.").arg(blockName));
            }
        }
    }
    // Block not already in list.
    blockVariantList.append(blockName);
    loadedBlocksVariant = QVariant(blockVariantList);

    p->setProperty(__loadedBlocks, loadedBlocksVariant);

    auto n = new BlockNode(blockName, p);
    const auto list = p->parse(n, QStringLiteral("endblock"));

    auto endBlock = p->takeNextToken();
    const QStringList acceptableBlocks{QStringLiteral("endblock"), QStringLiteral("endblock ") + blockName};
    if (!acceptableBlocks.contains(endBlock.content)) {
        p->invalidBlockTag(endBlock, QStringLiteral("endblock"), acceptableBlocks);
    }

    n->setNodeList(list);

    return n;
}

BlockNode::BlockNode(const QString &name, QObject *parent)
    : Node(parent)
    , m_name(name)
    , m_stream(nullptr)
{
    qRegisterMetaType<KTextTemplate::SafeString>("KTextTemplate::SafeString");
}

BlockNode::~BlockNode() = default;

void BlockNode::setNodeList(const NodeList &list) const
{
    m_list = list;
}

void BlockNode::render(OutputStream *stream, Context *c) const
{
    QVariant &variant = c->renderContext()->data(BLOCK_CONTEXT_KEY);
    auto blockContext = variant.value<BlockContext>();

    c->push();

    if (blockContext.isEmpty()) {
        m_context = c;
        m_stream = stream;
        c->insert(QStringLiteral("block"), QVariant::fromValue(const_cast<QObject *>(static_cast<const QObject *>(this))));
        m_list.render(stream, c);
        m_stream = nullptr;
    } else {
        auto block = static_cast<const BlockNode *>(blockContext.pop(m_name));
        variant.setValue(blockContext);
        auto push = block;
        if (!block)
            block = this;

        const auto list = block->m_list;

        block = new BlockNode(block->m_name, nullptr);
        block->setNodeList(list);
        block->m_context = c;
        block->m_stream = stream;
        c->insert(QStringLiteral("block"), QVariant::fromValue(const_cast<QObject *>(static_cast<const QObject *>(block))));
        list.render(stream, c);

        delete block;
        if (push) {
            blockContext.push(m_name, push);
            variant.setValue(blockContext);
        }
    }
    c->pop();
}

SafeString BlockNode::getSuper() const
{
    if (m_context->renderContext()->contains(BLOCK_CONTEXT_KEY)) {
        QVariant &variant = m_context->renderContext()->data(BLOCK_CONTEXT_KEY);
        const auto blockContext = variant.value<BlockContext>();
        auto block = blockContext.getBlock(m_name);
        if (block) {
            QString superContent;
            QTextStream superTextStream(&superContent);
            auto superStream = m_stream->clone(&superTextStream);
            const_cast<BlockNode *>(this)->render(superStream.data(), m_context);
            return markSafe(superContent);
        }
    }
    return {};
}

NodeList BlockNode::nodeList() const
{
    return m_list;
}

QString BlockNode::name() const
{
    return m_name;
}

#include "moc_block.cpp"
