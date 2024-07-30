/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef CUSTOM_TAGS_H
#define CUSTOM_TAGS_H

#include <KTextTemplate/TagLibraryInterface>

#include "rssfeed.h"

using namespace KTextTemplate;

class CustomPlugin : public QObject, public TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextTemplate::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.kde.KTextTemplate.TagLibraryInterface")
public:
    CustomPlugin(QObject *parent = 0)
        : QObject(parent)
    {
    }

    virtual QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = QString())
    {
        Q_UNUSED(name);
        QHash<QString, AbstractNodeFactory *> facts;
        facts.insert("rssfeed", new RssFeedNodeFactory());
        facts.insert("xmlrole", new XmlRoleNodeFactory());
        facts.insert("xmlns", new XmlNamespaceNodeFactory());
        return facts;
    }

    virtual QHash<QString, Filter *> filters(const QString &name = QString())
    {
        Q_UNUSED(name);
        QHash<QString, Filter *> _filters;
        _filters.insert("resize", new ResizeFilter);
        return _filters;
    }
};

#endif
