/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef TAGLIBRARYINTERFACE_H
#define TAGLIBRARYINTERFACE_H

#include "outputstream.h"

#include <QHash>
#include <QtPlugin>

namespace KTextTemplate
{
class AbstractNodeFactory;
class Filter;

/// @headerfile taglibraryinterface.h <KTextTemplate/TagLibraryInterface>

/**
  @brief The **%TagLibraryInterface** returns available tags and filters from
  libraries.

  This interface must be implemented in tag and filter libraries.

  The implementation will usually be very simple.

  @code
    class MyTagLibrary : public QObject, public TagLibraryInterface
    {
      Q_OBJECT
      Q_INTERFACES( KTextTemplate::TagLibraryInterface )
    public:
      MyTagLibrary( QObject *parent = {} )
          : QObject( parent ) {
      }

      QHash<QString, AbstractNodeFactory*>
      nodeFactories(const QString &name = {}) {
        Q_UNUSED( name );
        QHash<QString, AbstractNodeFactory*> nodeFactories;
        nodeFactories.insert( "mytag1", new MyTag1() );
        nodeFactories.insert( "mytag2", new MyTag2() );
        return nodeFactories;
      }

      QHash<QString, Filter*> filters( const QString &name = {} ) {
        Q_UNUSED( name );

        QHash<QString, Filter*> filters;

        filters.insert( "myfilter1", new MyFilter1() );
        filters.insert( "myfilter2", new MyFilter2() );

        return filters;
      }
    };
  @endcode

  @author Stephen Kelly <steveire@gmail.com>
*/
class TagLibraryInterface
{
public:
    virtual ~TagLibraryInterface()
    {
    }

    /**
      Returns the AbstractNodeFactory implementations available in this library.
    */
    virtual QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = {})
    {
        Q_UNUSED(name);
        static const QHash<QString, AbstractNodeFactory *> h;
        return h;
    };

    /**
      Returns the Filter implementations available in this library.
    */
    virtual QHash<QString, Filter *> filters(const QString &name = {})
    {
        Q_UNUSED(name);
        static const QHash<QString, Filter *> h;
        return h;
    };
};
}

Q_DECLARE_INTERFACE(KTextTemplate::TagLibraryInterface, "org.kde.KTextTemplate.TagLibraryInterface/1.0")

#endif
