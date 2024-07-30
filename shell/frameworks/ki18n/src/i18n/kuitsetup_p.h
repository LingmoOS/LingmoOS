/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUITSETUP_P_H
#define KUITSETUP_P_H

#include <QString>

class KuitFormatter;
class KuitFormatterPrivate;

namespace Kuit
{
/**
 * Convert &, ", ', <, > characters into XML entities
 * &amp;, &lt;, &gt;, &apos;, &quot;, respectively.
 */
QString escape(const QString &text);
}

/**
 * @internal
 * (used by KLocalizedString)
 *
 * KuitFormatter resolves KUIT markup in user interface text
 * into appropriate visual formatting.
 *
 * @author Chusslove Illich <caslav.ilic@gmx.net>
 * @short class for formatting KUIT markup in UI messages
 */
class KuitFormatter
{
public:
    /**
     * Constructor.
     *
     * @param language language to create the formatter for
     */
    KuitFormatter(const QString &language);

    /**
     * Transforms KUIT markup in the given text into visual formatting.
     * The appropriate visual formatting is decided based on
     * the context marker provided in the context string.
     *
     * @param domain translation domain from which the text was fetched
     * @param context context of the text (used if \p format == UndefinedFormat)
     * @param text text containing the KUIT markup
     * @param format target visual format
     * @param isArgument whether this text is inserted into an outer text
     */
    QString format(const QByteArray &domain, const QString &context, const QString &text, Kuit::VisualFormat format) const;

    /**
     * Destructor.
     */
    ~KuitFormatter();

private:
    KuitFormatter(const KuitFormatter &t);
    KuitFormatter &operator=(const KuitFormatter &t);

    KuitFormatterPrivate *d;
};

#endif
