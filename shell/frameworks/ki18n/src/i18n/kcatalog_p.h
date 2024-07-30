/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2012, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCATALOG_H
#define KCATALOG_H

#include "ki18n_export.h"

#include <QByteArray>
#include <QSet>
#include <QString>
#include <memory>

class KCatalogPrivate;

/**
 * This class abstracts a Gettext message catalog.
 * It takes care of needed Gettext bindings.
 *
 * @see KLocalizedString
 * @internal exported only for use in KI18nLocaleData.
 */
class KI18N_EXPORT KCatalog
{
public:
    /**
     * Constructor.
     *
     * @param name translation domain
     * @param language translation language
     */
    KCatalog(const QByteArray &domain, const QString &language);

    /**
     * Destructor.
     */
    ~KCatalog();

    /**
     * Get translation of the given message text.
     *
     * Do not pass empty message text.
     *
     * @param msgid message text
     *
     * @return translated message if found, <tt>QString()</tt> otherwise
     */
    QString translate(const QByteArray &msgid) const;

    /**
     * Get translation of the given message text with given context.
     *
     * Do not pass empty message text.
     *
     * @param msgctxt message context
     * @param msgid message text
     *
     * @return translated message if found, <tt>QString()</tt> otherwise
     */
    QString translate(const QByteArray &msgctxt, const QByteArray &msgid) const;

    /**
     * Get translation of given message with plural forms.
     *
     * Do not pass empty message text.
     *
     * @param msgid singular message text
     * @param msgid_plural plural message text
     * @param n number for which the plural form is needed
     *
     * @return translated message if found, <tt>QString()</tt> otherwise
     */
    QString translate(const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const;

    /**
     * Get translation of given message with plural forms with given context.
     *
     * Do not pass empty message text.
     *
     * @param msgctxt message context
     * @param msgid singular message text
     * @param msgid_plural plural message text
     * @param n number for which the plural form is needed
     *
     * @return translated message if found, <tt>QString()</tt> otherwise
     */
    QString translate(const QByteArray &msgctxt, const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const;

    /**
     * Find the locale directory for the given domain in the given language.
     *
     * @param domain translation domain
     * @param language language of the catalog
     *
     * @return the locale directory if found, <tt>QByteArray()</tt> otherwise.
     */
    static QString catalogLocaleDir(const QByteArray &domain, const QString &language);

    /**
     * Find the all languages for which the translation catalog
     * of given domain exists.
     *
     * @param domain translation domain
     *
     * @return set of language codes
     */
    static QSet<QString> availableCatalogLanguages(const QByteArray &domain);

    static void addDomainLocaleDir(const QByteArray &domain, const QString &path);

private:
    Q_DISABLE_COPY(KCatalog)

    std::unique_ptr<KCatalogPrivate> const d;
};

#endif
