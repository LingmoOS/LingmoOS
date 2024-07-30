/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "autoescape.h"

#include "exception.h"
#include "parser.h"

#include <QFile>

AutoescapeNodeFactory::AutoescapeNodeFactory() = default;

Node *AutoescapeNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = tagContent.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    if (expr.size() != 2) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("autoescape takes two arguments."));
    }

    auto strState = expr.at(1);
    int state;
    if (strState == QStringLiteral("on"))
        state = AutoescapeNode::On;
    else if (strState == QStringLiteral("off"))
        state = AutoescapeNode::Off;
    else {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("argument must be 'on' or 'off'"));
    }

    auto n = new AutoescapeNode(state, p);

    const auto list = p->parse(n, QStringLiteral("endautoescape"));
    p->removeNextToken();

    n->setList(list);

    return n;
}

AutoescapeNode::AutoescapeNode(int state, QObject *parent)
    : Node(parent)
    , m_state(state)
{
}

void AutoescapeNode::setList(const NodeList &list)
{
    m_list = list;
}

void AutoescapeNode::render(OutputStream *stream, Context *c) const
{
    const auto old_setting = c->autoEscape();
    c->setAutoEscape(m_state == On);
    m_list.render(stream, c);
    c->setAutoEscape(old_setting);
}

#include "moc_autoescape.cpp"
