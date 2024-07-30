/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_VARIABLE_H
#define KTEXTTEMPLATE_VARIABLE_H

#include "ktexttemplate_export.h"

#include <QVariant>

namespace KTextTemplate
{
class Context;

class VariablePrivate;

/// @headerfile variable.h <KTextTemplate/Variable>

/**
  @brief A container for static variables defined in Templates.

  This class is only relevant to Template tag authors.

  When processing a template tag in a AbstractNodeFactory implementation, it
  will sometimes make sense to process arguments to the tag as
  KTextTemplate::Variables. Note that usually they should be processed as
  FilterExpression objects instead.

  Arguments to the tag can be used to construct Variables, which may then be
  resolved into the objects they represent in the given Context in the render
  stage.

  @author Stephen Kelly <steveire@gmail.com>
*/
class KTEXTTEMPLATE_EXPORT Variable
{
public:
    /**
      Constructs an invalid **%Variable**
    */
    Variable();

    /**
      Creates a **%Variable** represented by the given @p var
    */
    explicit Variable(const QString &var);

    /**
      Copy constructor
    */
    Variable(const Variable &other);

    /**
      Destructor
    */
    ~Variable();

    /**
      Assignment operator.
    */
    Variable &operator=(const Variable &other);

    /**
      Returns whether this **%Variable** is valid.
    */
    bool isValid() const;

    /**
      Returns whether this **%Variable** evaluates to true with the Context @p c.
    */
    bool isTrue(Context *c) const;

    /**
      Resolves this **%Variable** with the Context @p c.
    */
    QVariant resolve(Context *c) const;

    /**
      Returns whether this **%Variable** is a constant in the Template. A constant
      is represented as a static string in the template

      @code
        Text content
        {% some_tag "constant" variable %}
      @endcode
    */
    bool isConstant() const;

    /**
      Returns whether this variable is localized, that is, if it is wrapped with
      _(). @see @ref i18n_l10n
     */
    bool isLocalized() const;

    /**
      Returns whether this variable is a literal string or number. A
      literal **%Variable** does not have any lookup components.
     */
    QVariant literal() const;

    /**
      Returns the lookup components of this **%Variable**.
     */
    QStringList lookups() const;

private:
    Q_DECLARE_PRIVATE(Variable)
    VariablePrivate *const d_ptr;
};
}

#endif
