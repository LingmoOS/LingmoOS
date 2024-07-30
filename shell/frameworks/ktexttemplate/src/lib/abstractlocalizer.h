/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_ABSTRACTLOCALIZER_H
#define KTEXTTEMPLATE_ABSTRACTLOCALIZER_H

#include "ktexttemplate_export.h"

#include <QLocale>
#include <QSharedPointer>
#include <QVariantList>

class QDateTime;

namespace KTextTemplate
{

/// @headerfile abstractlocalizer.h <KTextTemplate/AbstractLocalizer>

/**
  @brief Interface for implementing an internationalization system.

  This class defines an interface for accessing an internationalization, such as
  QLocale/QTranslator or gettext/KLocale.

  @author Stephen Kelly <steveire@gmail.com>
*/
class KTEXTTEMPLATE_EXPORT AbstractLocalizer
{
public:
    /**
     Constructor
    */
    AbstractLocalizer();

    /**
     Destructor
    */
    virtual ~AbstractLocalizer();

    /**
      Processes the data in @p variant into a localized string.

      The base implementation can localize numbers, dates and times, and
      strings.
    */
    virtual QString localize(const QVariant &variant) const;

    /**
      Returns the current locale as a string.
    */
    virtual QString currentLocale() const = 0;

    /**
      Makes this localizer use the locale specified by @p localeName for output.
    */
    virtual void pushLocale(const QString &localeName) = 0;

    /**
      Makes this localizer use the previous locale for output.
     */
    virtual void popLocale() = 0;

    /**
      Loads the @p catalog from @p path.
    */
    virtual void loadCatalog(const QString &path, const QString &catalog) = 0;

    /**
      Unloads the @p catalog
    */
    virtual void unloadCatalog(const QString &catalog) = 0;

    /**
      Localizes @p number
    */
    virtual QString localizeNumber(int number) const = 0;

    /**
      Localizes @p number
    */
    virtual QString localizeNumber(qreal number) const = 0;

    /**
      Localizes @p value as a monetary value in the currency specified by @p
      currencyCode.
    */
    virtual QString localizeMonetaryValue(qreal value, const QString &currencyCode = {}) const = 0;

    /**
      Localizes @p date with the specified @p formatType
    */
    virtual QString localizeDate(const QDate &date, QLocale::FormatType formatType = QLocale::ShortFormat) const = 0;

    /**
      Localizes @p time with the specified @p formatType
    */
    virtual QString localizeTime(const QTime &time, QLocale::FormatType formatType = QLocale::ShortFormat) const = 0;

    /**
      Localizes @p dateTime with the specified @p formatType
    */
    virtual QString localizeDateTime(const QDateTime &dateTime, QLocale::FormatType formatType = QLocale::ShortFormat) const = 0;

    /**
      Localizes @p string with the specified @p arguments for substitution
    */
    virtual QString localizeString(const QString &string, const QVariantList &arguments = {}) const = 0;

    /**
      Localizes @p string, disambiguated by @p context with the specified @p
      arguments for substitution
    */
    virtual QString localizeContextString(const QString &string, const QString &context, const QVariantList &arguments = {}) const = 0;

    /**
      Localizes @p string or its @p pluralForm with the specified @p arguments
      for substitution
    */
    virtual QString localizePluralString(const QString &string, const QString &pluralForm, const QVariantList &arguments = {}) const = 0;

    /**
      Localizes @p string or its @p pluralForm, disambiguated by @p context with
      the specified @p arguments for substitution
    */
    virtual QString
    localizePluralContextString(const QString &string, const QString &pluralForm, const QString &context, const QVariantList &arguments = {}) const = 0;

private:
    Q_DISABLE_COPY(AbstractLocalizer)
};
}

#endif
