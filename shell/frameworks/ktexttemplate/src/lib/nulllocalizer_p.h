/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_NULLLOCALIZER_P_H
#define KTEXTTEMPLATE_NULLLOCALIZER_P_H

#include "ktexttemplate_test_export.h"

#include "abstractlocalizer.h"

namespace KTextTemplate
{

class KTEXTTEMPLATE_TESTS_EXPORT NullLocalizer : public AbstractLocalizer
{
public:
    NullLocalizer();
    ~NullLocalizer() override;

    QString currentLocale() const override;
    void pushLocale(const QString &localeName) override;
    void popLocale() override;
    void loadCatalog(const QString &path, const QString &catalog) override;
    void unloadCatalog(const QString &catalog) override;

    QString localizeNumber(int number) const override;
    QString localizeNumber(qreal number) const override;
    QString localizeMonetaryValue(qreal value, const QString &currencyCode = {}) const override;
    QString localizeDate(const QDate &date, QLocale::FormatType formatType = QLocale::ShortFormat) const override;
    QString localizeTime(const QTime &time, QLocale::FormatType formatType = QLocale::ShortFormat) const override;
    QString localizeDateTime(const QDateTime &dateTime, QLocale::FormatType formatType = QLocale::ShortFormat) const override;
    QString localizeString(const QString &string, const QVariantList &arguments = {}) const override;
    QString localizeContextString(const QString &string, const QString &context, const QVariantList &arguments = {}) const override;
    QString
    localizePluralContextString(const QString &string, const QString &pluralForm, const QString &context, const QVariantList &arguments = {}) const override;
    QString localizePluralString(const QString &string, const QString &pluralForm, const QVariantList &arguments = {}) const override;

private:
    Q_DISABLE_COPY(NullLocalizer)
};
}

#endif
