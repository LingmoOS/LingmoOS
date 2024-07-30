/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "extends.h"

#include "block.h"
#include "blockcontext.h"
#include "engine.h"
#include "exception.h"
#include "nodebuiltins_p.h"
#include "parser.h"
#include "rendercontext.h"
#include "util.h"

using namespace KTextTemplate;

ExtendsNodeFactory::ExtendsNodeFactory(QObject *parent)
    : AbstractNodeFactory(parent)
{
}

Node *ExtendsNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    const auto expr = smartSplit(tagContent);

    if (expr.size() != 2)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: Include tag takes only one argument"));

    FilterExpression fe(expr.at(1), p);

    auto n = new ExtendsNode(fe, p);

    auto t = qobject_cast<TemplateImpl *>(p->parent());

    if (!t)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Extends tag is not in a template."));

    const auto nodeList = p->parse(t);
    n->setNodeList(nodeList);

    if (t->findChildren<ExtendsNode *>().size() > 1) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Extends tag may only appear once in a template."));
    }

    return n;
}

ExtendsNode::ExtendsNode(const FilterExpression &fe, QObject *parent)
    : Node(parent)
    , m_filterExpression(fe)
{
}

ExtendsNode::~ExtendsNode() = default;

static QHash<QString, BlockNode *> createNodeMap(const QList<BlockNode *> &list)
{
    QHash<QString, BlockNode *> map;

    auto it = list.constBegin();
    const auto end = list.constEnd();

    for (; it != end; ++it) {
        map.insert((*it)->name(), *it);
    }

    return map;
}

void ExtendsNode::setNodeList(const NodeList &list)
{
    m_list = list;

    const auto blockList = m_list.findChildren<BlockNode *>();
    m_blocks = createNodeMap(blockList);
}

Template ExtendsNode::getParent(Context *c) const
{
    const auto parentVar = m_filterExpression.resolve(c);
    if (parentVar.userType() == qMetaTypeId<KTextTemplate::Template>()) {
        return parentVar.value<Template>();
    }

    QString parentName = getSafeString(parentVar);

    auto ti = containerTemplate();

    const auto t = ti->engine()->loadByName(parentName);

    if (!t)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Template not found %1").arg(parentName));

    if (t->error())
        throw KTextTemplate::Exception(t->error(), t->errorString());

    return t;
}

void ExtendsNode::render(OutputStream *stream, Context *c) const
{
    const auto parentTemplate = getParent(c);

    if (!parentTemplate) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Cannot load template."));
    }

    QVariant &variant = c->renderContext()->data(nullptr);
    auto blockContext = variant.value<BlockContext>();
    blockContext.addBlocks(m_blocks);
    variant.setValue(blockContext);

    const auto nodeList = parentTemplate->nodeList();

    const auto parentBlocks = createNodeMap(parentTemplate->findChildren<BlockNode *>());

    for (auto n : nodeList) {
        auto tn = qobject_cast<TextNode *>(n);
        if (!tn) {
            auto en = qobject_cast<ExtendsNode *>(n);
            if (!en) {
                blockContext.addBlocks(parentBlocks);
                variant.setValue(blockContext);
            }
            break;
        }
    }
    variant.setValue(blockContext);
    parentTemplate->nodeList().render(stream, c);

    auto nodes = parentTemplate->findChildren<BlockNode *>();
    blockContext.remove(nodes);
    variant.setValue(blockContext);
}

void ExtendsNode::appendNode(Node *node)
{
    m_list.append(node);
    node->setParent(parent());
}

#include "moc_extends.cpp"
