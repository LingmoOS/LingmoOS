/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "templatetag.h"

#include "../lib/exception.h"
#include "ktexttemplate_tags_p.h"
#include "parser.h"

TemplateTagNodeFactory::TemplateTagNodeFactory() = default;

Node *TemplateTagNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = tagContent.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    expr.takeAt(0);
    if (expr.isEmpty()) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("'templatetag' statement takes one argument"));
    }

    const auto name = expr.constFirst();

    if (!TemplateTagNode::isKeyword(name)) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Not a template tag"));
    }

    return new TemplateTagNode(name, p);
}

TemplateTagNode::TemplateTagNode(const QString &name, QObject *parent)
    : Node(parent)
    , m_name(name)
{
}

static QHash<QString, QString> getKeywordMap()
{
    QHash<QString, QString> map;
    map.insert(QStringLiteral("openblock"), QLatin1String(BLOCK_TAG_START));
    map.insert(QStringLiteral("closeblock"), QLatin1String(BLOCK_TAG_END));
    map.insert(QStringLiteral("openvariable"), QLatin1String(VARIABLE_TAG_START));
    map.insert(QStringLiteral("closevariable"), QLatin1String(VARIABLE_TAG_END));
    map.insert(QStringLiteral("openbrace"), QChar::fromLatin1('{'));
    map.insert(QStringLiteral("closebrace"), QChar::fromLatin1('}'));
    map.insert(QStringLiteral("opencomment"), QLatin1String(COMMENT_TAG_START));
    map.insert(QStringLiteral("closecomment"), QLatin1String(COMMENT_TAG_END));
    return map;
}

bool TemplateTagNode::isKeyword(const QString &name)
{
    static auto map = getKeywordMap();
    return map.contains(name);
}

void TemplateTagNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(c)
    static auto map = getKeywordMap();
    (*stream) << map.value(m_name);
}

#include "moc_templatetag.cpp"
