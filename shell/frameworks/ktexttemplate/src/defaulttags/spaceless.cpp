/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "spaceless.h"

#include <QRegularExpression>

#include "parser.h"
#include "util.h"

SpacelessNodeFactory::SpacelessNodeFactory() = default;

Node *SpacelessNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    Q_UNUSED(tagContent)
    auto n = new SpacelessNode(p);
    const auto list = p->parse(n, QStringLiteral("endspaceless"));
    n->setList(list);
    p->removeNextToken();
    return n;
}

SpacelessNode::SpacelessNode(QObject *parent)
    : Node(parent)
{
}

void SpacelessNode::setList(const NodeList &nodeList)
{
    m_nodeList = nodeList;
}

QString SpacelessNode::stripSpacesBetweenTags(const QString &input)
{
    auto stripped = input;

    static const QRegularExpression re(QStringLiteral(">\\s+<"));
    stripped.replace(re, QStringLiteral("><"));
    return stripped;
}

void SpacelessNode::render(OutputStream *stream, Context *c) const
{
    QString output;
    QTextStream textStream(&output);
    auto temp = stream->clone(&textStream);
    m_nodeList.render(temp.data(), c);
    (*stream) << markSafe(stripSpacesBetweenTags(output.trimmed()));
}

#include "moc_spaceless.cpp"
