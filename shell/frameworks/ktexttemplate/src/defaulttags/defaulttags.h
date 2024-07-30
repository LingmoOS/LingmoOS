/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef DEFAULTTAGS_H
#define DEFAULTTAGS_H

#include "autoescape.h"
#include "comment.h"
#include "cycle.h"
#include "debug.h"
#include "filtertag.h"
#include "firstof.h"
#include "for.h"
#include "if.h"
#include "ifchanged.h"
#include "ifequal.h"
#include "load.h"
#include "mediafinder.h"
#include "now.h"
#include "range.h"
#include "regroup.h"
#include "spaceless.h"
#include "templatetag.h"
#include "widthratio.h"
#include "with.h"

#include "node.h"
#include "taglibraryinterface.h"

using namespace KTextTemplate;

class DefaultTagLibrary : public QObject, public TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextTemplate::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.kde.KTextTemplate.TagLibraryInterface")
public:
    explicit DefaultTagLibrary(QObject *parent = {})
        : QObject(parent)
    {
    }

    QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = {}) override
    {
        Q_UNUSED(name);

        QHash<QString, AbstractNodeFactory *> nodeFactories;

        nodeFactories.insert(QStringLiteral("autoescape"), new AutoescapeNodeFactory());
        nodeFactories.insert(QStringLiteral("comment"), new CommentNodeFactory());
        nodeFactories.insert(QStringLiteral("cycle"), new CycleNodeFactory());
        nodeFactories.insert(QStringLiteral("debug"), new DebugNodeFactory());
        nodeFactories.insert(QStringLiteral("filter"), new FilterNodeFactory());
        nodeFactories.insert(QStringLiteral("firstof"), new FirstOfNodeFactory());
        nodeFactories.insert(QStringLiteral("for"), new ForNodeFactory());
        nodeFactories.insert(QStringLiteral("if"), new IfNodeFactory());
        nodeFactories.insert(QStringLiteral("ifchanged"), new IfChangedNodeFactory());
        nodeFactories.insert(QStringLiteral("ifequal"), new IfEqualNodeFactory());
        nodeFactories.insert(QStringLiteral("ifnotequal"), new IfNotEqualNodeFactory());
        nodeFactories.insert(QStringLiteral("load"), new LoadNodeFactory());
        nodeFactories.insert(QStringLiteral("media_finder"), new MediaFinderNodeFactory());
        nodeFactories.insert(QStringLiteral("now"), new NowNodeFactory());
        nodeFactories.insert(QStringLiteral("range"), new RangeNodeFactory());
        nodeFactories.insert(QStringLiteral("regroup"), new RegroupNodeFactory());
        nodeFactories.insert(QStringLiteral("spaceless"), new SpacelessNodeFactory());
        nodeFactories.insert(QStringLiteral("templatetag"), new TemplateTagNodeFactory());
        nodeFactories.insert(QStringLiteral("widthratio"), new WidthRatioNodeFactory());
        nodeFactories.insert(QStringLiteral("with"), new WithNodeFactory());

        return nodeFactories;
    }
};

#endif
