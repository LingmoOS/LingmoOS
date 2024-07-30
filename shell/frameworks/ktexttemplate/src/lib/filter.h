/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

// krazy:excludeall=dpointer

#ifndef KTEXTTEMPLATE_FILTER_H
#define KTEXTTEMPLATE_FILTER_H

#include "ktexttemplate_export.h"
#include "outputstream.h"
#include "safestring.h"

#include <QSharedPointer>
#include <QStringList>
#include <QVariant>

namespace KTextTemplate
{

/// @headerfile filter.h <KTextTemplate/Filter>

/**
  @brief Base class for all filters.

  The **%Filter** class can be implemented in plugin libraries to make
  additional functionality available to templates.

  Developers are required only to implement the @ref doFilter method and
  integrate the filter as part of a custom plugin, but will never create or
  access filters directly in application code.

  The FilterExpression class is the access interface to a chain of **%Filter**
  objects.

  The @ref escape and @ref conditionalEscape methods are available for escaping
  data where needed.

  @see @ref filters

  @author Stephen Kelly <steveire@gmail.com>
*/
class KTEXTTEMPLATE_EXPORT Filter
{
public:
    /**
      Destructor.
    */
    virtual ~Filter();

#ifndef K_DOXYGEN
    /**
      FilterExpression makes it possible to access stream methods like escape
      while resolving.
    */
    void setStream(OutputStream *stream);
#endif

    /**
      Escapes and returns @p input. The OutputStream::escape method is used to
      escape @p input.
    */
    SafeString escape(const QString &input) const;

    /**
      Escapes and returns @p input. The OutputStream::escape method is used to
      escape @p input.
    */
    SafeString escape(const SafeString &input) const;

    /**
      Escapes @p input if not already safe from further escaping and returns it.
      The OutputStream::escape method is used to escape @p input.
    */
    SafeString conditionalEscape(const SafeString &input) const;

    /**
      Reimplement to filter @p input given @p argument.

      @p autoescape determines whether the autoescape feature is currently on or
      off. Most filters will not use this.

      @see @ref autoescaping
    */
    virtual QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const = 0;

    /**
      Reimplement to return whether this filter is safe.
    */
    virtual bool isSafe() const;

private:
#ifndef K_DOXYGEN
    OutputStream *m_stream;
#endif
};
}

#endif
