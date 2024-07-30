/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KDE_lOCALIZER_H
#define KDE_lOCALIZER_H

#include <KTextTemplate/Abstractlocalizer>

class KLocale;

class QStringList;

class KDELocalizerPrivate;

class KDELocalizer : public KTextTemplate::AbstractLocalizer
{
public:
    KDELocalizer(KLocale *locale = 0);
    virtual ~KDELocalizer();

    // Right to left?

    void insertApplicationCatalogs(const QStringList &catalogs);

    virtual QString currentLocale() const;
    virtual void pushLocale(const QString &localeName);
    virtual void popLocale();
    virtual void loadCatalog(const QString &path, const QString &catalog);
    virtual void unloadCatalog(const QString &catalog);

    virtual QString localizeNumber(int number) const;
    virtual QString localizeNumber(qreal number) const;
    virtual QString localizeMonetaryValue(qreal value, const QString &currencyCode = QString()) const;
    virtual QString localizeDate(const QDate &date, QLocale::FormatType formatType = QLocale::ShortFormat) const;
    virtual QString localizeTime(const QTime &time, QLocale::FormatType formatType = QLocale::ShortFormat) const;
    virtual QString localizeDateTime(const QDateTime &dateTime, QLocale::FormatType formatType = QLocale::ShortFormat) const;
    virtual QString localizeString(const QString &string, const QVariantList &arguments = QVariantList()) const;
    virtual QString localizeContextString(const QString &string, const QString &context, const QVariantList &arguments = QVariantList()) const;
    virtual QString
    localizePluralContextString(const QString &string, const QString &pluralForm, const QString &context, const QVariantList &arguments = QVariantList()) const;
    virtual QString localizePluralString(const QString &string, const QString &pluralForm, const QVariantList &arguments = QVariantList()) const;

private:
    Q_DECLARE_PRIVATE(KDELocalizer)
    KDELocalizerPrivate *const d_ptr;
};

#endif