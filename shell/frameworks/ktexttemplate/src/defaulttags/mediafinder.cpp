/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "mediafinder.h"

#include "engine.h"
#include "exception.h"
#include "parser.h"
#include "util.h"

#include <QUrl>

MediaFinderNodeFactory::MediaFinderNodeFactory() = default;

KTextTemplate::Node *MediaFinderNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() <= 1) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("'media_finder' tag requires at least one argument"));
    }
    expr.takeAt(0);

    return new MediaFinderNode(getFilterExpressionList(expr, p), p);
}

MediaFinderNode::MediaFinderNode(const QList<FilterExpression> &mediaExpressionList, QObject *parent)
    : Node(parent)
    , m_mediaExpressionList(mediaExpressionList)
{
}

void MediaFinderNode::render(OutputStream *stream, Context *c) const
{
    auto t = containerTemplate();
    auto engine = t->engine();

    for (const FilterExpression &fe : m_mediaExpressionList) {
        if (fe.isTrue(c)) {
            const auto fileUrl = engine->mediaUri(getSafeString(fe.resolve(c)));
            if (fileUrl.second.isEmpty())
                continue;
            const auto uri = QUrl::fromLocalFile(fileUrl.first).toString();
            c->addExternalMedia(uri, fileUrl.second);
            if (c->urlType() == Context::AbsoluteUrls)
                streamValueInContext(stream, uri, c);
            else if (!c->relativeMediaPath().isEmpty())
                streamValueInContext(stream, QVariant(c->relativeMediaPath() + QLatin1Char('/')), c);
            streamValueInContext(stream, fileUrl.second, c);
            return;
        }
    }
}

#include "moc_mediafinder.cpp"
