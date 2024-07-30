/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_CACHINGLOADERDECORATOR_H
#define KTEXTTEMPLATE_CACHINGLOADERDECORATOR_H

#include "templateloader.h"

#include "ktexttemplate_export.h"

namespace KTextTemplate
{

class CachingLoaderDecoratorPrivate;

/// @headerfile cachingloaderdecorator.h <KTextTemplate/CachingLoaderDecorator>

/**
  @brief Implements a loader decorator which caches compiled Template objects.

  The **%CachingLoaderDecorator** can be used with any implementation of
  KTextTemplate::AbstractTemplateLoader.

  @code
    auto loader = QSharedPointer<KTextTemplate::FileSystemTemplateLoader>::create();
    loader->setTemplateDirs({"/path/one", "/path/two"});

    auto cache =
        QSharedPointer<KTextTemplate::CachingLoaderDecorator>::create( loader );
    m_engine->addTemplateLoader( cache );
  @endcode

  The effect is that templates do not need to be loaded from the filesystem and
  compiled into Template objects on each access, but may be returned from the
  cache instead.

  This can be significant if loading Templates in a loop, or loading the same
  Template very often in an application.

  @code
    <ul>
    {% for item in list %}
      <li>{% include "itemtemplate.html" %}</li>
    {% endfor %}
    </ul>
  @endcode

  If the loading of Templates is a bottleneck in an application, it may make
  sense to use the caching decorator.

  @author Stephen Kelly <steveire@gmail.com>
 */
class KTEXTTEMPLATE_EXPORT CachingLoaderDecorator : public AbstractTemplateLoader
{
public:
    /**
      Constructor
    */
    CachingLoaderDecorator(QSharedPointer<AbstractTemplateLoader> loader);

    /**
      Destructor
    */
    ~CachingLoaderDecorator() override;

    bool canLoadTemplate(const QString &name) const override;

    std::pair<QString, QString> getMediaUri(const QString &fileName) const override;

    Template loadByName(const QString &name, const KTextTemplate::Engine *engine) const override;

    /**
      Clears the Templates objects cached in the decorator.
     */
    void clear();

    /**
      Returns the number of Template objects cached in the decorator.
     */
    int size() const;

    /**
      Returns whether the cache is empty.
     */
    bool isEmpty() const;

private:
    Q_DECLARE_PRIVATE(CachingLoaderDecorator)
    CachingLoaderDecoratorPrivate *const d_ptr;
};
}

#endif
