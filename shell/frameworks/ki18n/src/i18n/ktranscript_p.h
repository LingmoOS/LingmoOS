/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTRANSCRIPT_P_H
#define KTRANSCRIPT_P_H

#include <QHash>
#include <QList>
#include <QStringList>
#include <QVariant>

/**
 * @internal
 * (used by KLocalizedString)
 *
 * @c KTranscript provides support for programmable translations.
 * The class is abstract in order to facilitate dynamic loading.
 *
 * @author Chusslove Illich <caslav.ilic@gmx.net>
 * @short class for supporting programmable translations
 */
class KTranscript
{
public:
    /**
     * Evaluates interpolation.
     *
     * @param argv list of interpolation tokens
     * @param lang language of the translation
     * @param ctry locale country
     * @param msgctxt message context
     * @param dynctxt dynamic context
     * @param msgid original message
     * @param subs substitutions for message placeholders
     * @param vals values that were formatted to substitutions
     * @param ftrans finalized ordinary translation
     * @param mods scripting modules to load; the list is cleared after loading
     * @param error set to the message detailing the problem, if the script
                    failed; set to empty otherwise
     * @param fallback set to true if the script requested fallback to ordinary
                       translation; set to false otherwise
     * @return resolved interpolation if evaluation succeeded,
     *         empty string otherwise
     */
    virtual QString eval(const QList<QVariant> &argv,
                         const QString &lang,
                         const QString &ctry,
                         const QString &msgctxt,
                         const QHash<QString, QString> &dynctxt,
                         const QString &msgid,
                         const QStringList &subs,
                         const QList<QVariant> &vals,
                         const QString &ftrans,
                         QList<QStringList> &mods,
                         QString &error,
                         bool &fallback) = 0;

    /**
     * Returns the list of calls to execute an all messages after the
     * interpolations are done, as evaluations with no parameters.
     *
     * @param lang language of the translation
     * @return list of post calls
     */
    virtual QStringList postCalls(const QString &lang) = 0;

    /**
     * Destructor.
     */
    virtual ~KTranscript()
    {
    }
};

#ifdef KTRANSCRIPT_TESTBUILD
KTranscript *autotestCreateKTranscriptImp();
void autotestDestroyKTranscriptImp();
#endif

#endif
