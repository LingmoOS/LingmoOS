/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef LOADERTAGS_H
#define LOADERTAGS_H

#include "node.h"
#include "taglibraryinterface.h"

#include "block.h"
#include "extends.h"
#include "include.h"

using namespace KTextTemplate;

class LoaderTagLibrary : public QObject, public TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextTemplate::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.kde.KTextTemplate.TagLibraryInterface")
public:
    LoaderTagLibrary()
    {
    }

    QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = {}) override
    {
        Q_UNUSED(name);

        QHash<QString, AbstractNodeFactory *> nodeFactories;
        nodeFactories.insert(QStringLiteral("block"), new BlockNodeFactory());
        nodeFactories.insert(QStringLiteral("extends"), new ExtendsNodeFactory());
        nodeFactories.insert(QStringLiteral("include"), new IncludeNodeFactory());
        return nodeFactories;
    }
};

#endif
