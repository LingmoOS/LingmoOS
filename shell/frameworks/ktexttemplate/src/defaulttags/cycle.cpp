/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "cycle.h"

#include "../lib/exception.h"
#include "parser.h"
#include "rendercontext.h"

static const char _namedCycleNodes[] = "_namedCycleNodes";

CycleNodeFactory::CycleNodeFactory() = default;

Node *CycleNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() < 2) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("%1 expects at least one argument").arg(expr.first()));
    }

    if (expr.at(1).contains(QLatin1Char(','))) {
        auto csvlist = expr.at(1).split(QLatin1Char(','));
        expr.removeAt(1);
        for (auto i = 0; i < csvlist.size(); ++i) {
            expr.insert(i + 1, QChar::fromLatin1('"') + csvlist.at(i) + QChar::fromLatin1('"'));
        }
    }

    if (expr.size() == 2) {
        // {% cycle var %}
        auto name = expr.at(1);
        auto cycleNodes = p->property(_namedCycleNodes);
        if (cycleNodes.userType() != qMetaTypeId<QVariantHash>()) {
            throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("No named cycles in template. '%1' is not defined").arg(name));
        }
        auto hash = cycleNodes.value<QVariantHash>();
        if (!hash.contains(name)) {
            throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Node not found: %1").arg(name));
        }
        auto nodeVariant = hash.value(name);
        Q_ASSERT(nodeVariant.canConvert<Node *>());
        return nodeVariant.value<Node *>();
    }

    auto exprSize = expr.size();
    if (exprSize > 4 && expr.at(exprSize - 2) == QStringLiteral("as")) {
        // {% cycle "foo" "bar" "bat" as var %}
        auto name = expr.at(exprSize - 1);
        auto list = expr.mid(1, exprSize - 3);
        auto node = new CycleNode(getFilterExpressionList(list, p), name, p);
        auto hashVariant = p->property(_namedCycleNodes);
        QVariantHash hash;
        if (hashVariant.userType() == qMetaTypeId<QVariantHash>()) {
            hash = hashVariant.value<QVariantHash>();
        }
        hash.insert(name, QVariant::fromValue(node));
        p->setProperty(_namedCycleNodes, QVariant(hash));
        return node;
    }
    auto list = expr.mid(1, exprSize - 1);
    return new CycleNode(getFilterExpressionList(list, p), QString(), p);
}

CycleNode::CycleNode(const QList<FilterExpression> &list, const QString &name, QObject *parent)
    : Node(parent)
    , m_list(list)
    , m_variableIterator(list)
    , m_name(name)
{
}

void CycleNode::render(OutputStream *stream, Context *c) const
{
    QVariant &variant = c->renderContext()->data(this);

    FilterExpressionRotator rotator;

    if (variant.isValid())
        rotator = variant.value<FilterExpressionRotator>();
    else
        rotator = FilterExpressionRotator(m_list);

    QString value;
    QTextStream textStream(&value);
    auto temp = stream->clone(&textStream);

    rotator.next().resolve(temp.data(), c).toString();

    variant.setValue(rotator);

    if (!m_name.isEmpty()) {
        c->insert(m_name, value);
    }
    (*stream) << value;
}

#include "moc_cycle.cpp"
