/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Lars Knoll <knoll@kde.org>
    SPDX-FileCopyrightText: 2001, 2003, 2004, 2005, 2006 Nicolas GOUTTE <goutte@kde.org>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kcharsets.h"
#include "kcharsets_p.h"
#include "kcodecs_debug.h"

#include <kentities.h>

#include <QHash>

#include <algorithm>
#include <assert.h>

/*
 * The encoding names (like "ISO 8859-1") in this list are user-visible,
 * and should be mostly uppercase.
 * Generate with generate_string_table.pl (located in kde-dev-scripts),
 * input data:
ISO 8859-1
i18n:Western European
ISO 8859-15
i18n:Western European
ISO 8859-14
i18n:Western European
cp 1252
i18n:Western European
IBM850
i18n:Western European
ISO 8859-2
i18n:Central European
ISO 8859-3
i18n:Central European
ISO 8859-4
i18n:Baltic
ISO 8859-13
i18n:Baltic
ISO 8859-16
i18n:South-Eastern Europe
cp 1250
i18n:Central European
cp 1254
i18n:Turkish
cp 1257
i18n:Baltic
KOI8-R
i18n:Cyrillic
ISO 8859-5
i18n:Cyrillic
cp 1251
i18n:Cyrillic
KOI8-U
i18n:Cyrillic
IBM866
i18n:Cyrillic
Big5
i18n:Chinese Traditional
Big5-HKSCS
i18n:Chinese Traditional
GB18030
i18n:Chinese Simplified
GBK
i18n:Chinese Simplified
GB2312
i18n:Chinese Simplified
EUC-KR
i18n:Korean
windows-949
i18n:Korean
sjis
i18n:Japanese
ISO-2022-JP
i18n:Japanese
EUC-JP
i18n:Japanese
ISO 8859-7
i18n:Greek
cp 1253
i18n:Greek
ISO 8859-6
i18n:Arabic
cp 1256
i18n:Arabic
ISO 8859-8
i18n:Hebrew
ISO 8859-8-I
i18n:Hebrew
cp 1255
i18n:Hebrew
ISO 8859-9
i18n:Turkish
TIS620
i18n:Thai
ISO 8859-11
i18n:Thai
UTF-8
i18n:Unicode
UTF-16
i18n:Unicode
utf7
i18n:Unicode
ucs2
i18n:Unicode
ISO 10646-UCS-2
i18n:Unicode
windows-1258
i18n:Other
IBM874
i18n:Other
TSCII
i18n:Other
 */
/*
 * Notes about the table:
 *
 * - The following entries were disabled and removed from the table:
ibm852
i18n:Central European
pt 154
i18n:Cyrillic              // ### TODO "PT 154" seems to have been removed from Qt
 *
 * - ISO 8559-11 is the deprecated name of TIS-620
 * - utf7 is not in Qt
 * - UTF-16 is duplicated as "ucs2" and "ISO 10646-UCS-2"
 * - windows-1258: TODO
 * - IBM874: TODO
 * - TSCII: TODO
 */

/*
 * This redefines the QT_TRANSLATE_NOOP3 macro provided by Qt to indicate that
 * statically initialised text should be translated so that it expands to just
 * the string that should be translated, making it possible to use it in the
 * single string construct below.
 */
#undef QT_TRANSLATE_NOOP3
#define QT_TRANSLATE_NOOP3(a, b, c) b

/*
 * THE FOLLOWING CODE IS GENERATED. PLEASE DO NOT EDIT BY HAND.
 * The script used was generate_string_table.pl which can be found in kde-dev-scripts.
 * It was then edited to use QT_TRANSLATE_NOOP3 instead of I18N_NOOP.
 */

static const char language_for_encoding_string[] =
    "ISO 8859-1\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Western European", "@item Text character set")"\0"
    "ISO 8859-15\0"
    "ISO 8859-14\0"
    "cp 1252\0"
    "IBM850\0"
    "ISO 8859-2\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Central European", "@item Text character set")"\0"
    "ISO 8859-3\0"
    "ISO 8859-4\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Baltic", "@item Text character set")"\0"
    "ISO 8859-13\0"
    "ISO 8859-16\0"
    QT_TRANSLATE_NOOP3("KCharsets", "South-Eastern Europe", "@item Text character set")"\0"
    "cp 1250\0"
    "cp 1254\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Turkish", "@item Text character set")"\0"
    "cp 1257\0"
    "KOI8-R\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Cyrillic", "@item Text character set")"\0"
    "ISO 8859-5\0"
    "cp 1251\0"
    "KOI8-U\0"
    "IBM866\0"
    "Big5\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Chinese Traditional", "@item Text character set")"\0"
    "Big5-HKSCS\0"
    "GB18030\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Chinese Simplified", "@item Text character set")"\0"
    "GBK\0"
    "GB2312\0"
    "EUC-KR\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Korean", "@item Text character set")"\0"
    "windows-949\0"
    "sjis\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Japanese", "@item Text character set")"\0"
    "ISO-2022-JP\0"
    "EUC-JP\0"
    "ISO 8859-7\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Greek", "@item Text character set")"\0"
    "cp 1253\0"
    "ISO 8859-6\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Arabic", "@item Text character set")"\0"
    "cp 1256\0"
    "ISO 8859-8\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Hebrew", "@item Text character set")"\0"
    "ISO 8859-8-I\0"
    "cp 1255\0"
    "ISO 8859-9\0"
    "TIS620\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Thai", "@item Text character set")"\0"
    "ISO 8859-11\0"
    "UTF-8\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Unicode", "@item Text character set")"\0"
    "UTF-16\0"
    "utf7\0"
    "ucs2\0"
    "ISO 10646-UCS-2\0"
    "windows-1258\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Other", "@item Text character set")"\0"
    "IBM874\0"
    "TSCII\0"
    "\0";

static const int language_for_encoding_indices[] = {
    0,   11,  28,  11,  40,  11,  52,  11,  60,  11,  67,  78,  95,  78,  106, 117, 124, 117, 136, 148, 169, 78,  177, 185, 193, 117, 201, 208, 217, 208, 228,
    208, 236, 208, 243, 208, 250, 255, 275, 255, 286, 294, 313, 294, 317, 294, 324, 331, 338, 331, 350, 355, 364, 355, 376, 355, 383, 394, 400, 394, 408, 419,
    426, 419, 434, 445, 452, 445, 465, 445, 473, 185, 484, 491, 496, 491, 508, 514, 522, 514, 529, 514, 534, 514, 539, 514, 555, 568, 574, 568, 581, 568, -1};

/*
 * GENERATED CODE ENDS HERE
 */

struct KCharsetsSingletonPrivate {
    KCharsets instance;
};

Q_GLOBAL_STATIC(KCharsetsSingletonPrivate, globalCharsets)

// search an array of items index/data, find first matching index
// and return data, or return 0
static inline const char *kcharsets_array_search(const char *start, const int *indices, const char *entry)
{
    for (int i = 0; indices[i] != -1; i += 2) {
        if (qstrcmp(start + indices[i], entry) == 0) {
            return start + indices[i + 1];
        }
    }
    return nullptr;
}

// --------------------------------------------------------------------------

KCharsets::KCharsets()
    : d(new KCharsetsPrivate)
{
}

KCharsets::~KCharsets() = default;

QChar KCharsets::fromEntity(QStringView str)
{
    QChar res = QChar::Null;

    if (str.isEmpty()) {
        return QChar::Null;
    }

    int pos = 0;
    if (str[pos] == QLatin1Char('&')) {
        pos++;
    }

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == QLatin1Char('#') && str.length() - pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == QLatin1Char('x') || str[pos] == QLatin1Char('X')) {
            pos++;
            // '&#x0000', hexadecimal character reference
            const auto tmp = str.mid(pos);
            res = QChar(tmp.toInt(&ok, 16));
        } else {
            //  '&#0000', decimal character reference
            const auto tmp = str.mid(pos);
            res = QChar(tmp.toInt(&ok, 10));
        }
        if (ok) {
            return res;
        } else {
            return QChar::Null;
        }
    }

    const QByteArray raw(str.toLatin1());
    const entity *e = KCodecsEntities::kde_findEntity(raw.data(), raw.length());

    if (!e) {
        // qCDebug(KCODECS_LOG) << "unknown entity " << str <<", len = " << str.length();
        return QChar::Null;
    }
    // qCDebug(KCODECS_LOG) << "got entity " << str << " = " << e->code;

    return QChar(e->code);
}

QChar KCharsets::fromEntity(QStringView str, int &len)
{
    // entities are never longer than 8 chars... we start from
    // that length and work backwards...
    len = 8;
    while (len > 0) {
        const auto tmp = str.left(len);
        QChar res = fromEntity(tmp);
        if (res != QChar::Null) {
            return res;
        }
        len--;
    }
    return QChar::Null;
}

QString KCharsets::toEntity(const QChar &ch)
{
    return QString::asprintf("&#0x%x;", ch.unicode());
}

QString KCharsets::resolveEntities(const QString &input)
{
    QString text = input;
    const QChar *p = text.unicode();
    const QChar *end = p + text.length();
    const QChar *ampersand = nullptr;
    bool scanForSemicolon = false;

    for (; p < end; ++p) {
        const QChar ch = *p;

        if (ch == QLatin1Char('&')) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if (ch != QLatin1Char(';') || scanForSemicolon == false) {
            continue;
        }

        assert(ampersand);

        scanForSemicolon = false;

        const QChar *entityBegin = ampersand + 1;

        const uint entityLength = p - entityBegin;
        if (entityLength == 0) {
            continue;
        }

        const QChar entityValue = KCharsets::fromEntity(QStringView(entityBegin, entityLength));
        if (entityValue.isNull()) {
            continue;
        }

        const uint ampersandPos = ampersand - text.unicode();

        text[(int)ampersandPos] = entityValue;
        text.remove(ampersandPos + 1, entityLength + 1);
        p = text.unicode() + ampersandPos;
        end = text.unicode() + text.length();
        ampersand = nullptr;
    }

    return text;
}

QStringList KCharsets::availableEncodingNames() const
{
    QStringList available;
    for (const int *p = language_for_encoding_indices; *p != -1; p += 2) {
        available.append(QString::fromUtf8(language_for_encoding_string + *p));
    }
    available.sort();
    return available;
}

QString KCharsets::descriptionForEncoding(QStringView encoding) const
{
    const char *lang = kcharsets_array_search(language_for_encoding_string, language_for_encoding_indices, encoding.toUtf8().data());
    if (lang) {
        return tr("%1 ( %2 )", "@item %1 character set, %2 encoding").arg(tr(lang, "@item Text character set"), encoding);
    } else {
        return tr("Other encoding (%1)", "@item").arg(encoding);
    }
}

QString KCharsets::encodingForName(const QString &descriptiveName) const
{
    const int left = descriptiveName.lastIndexOf(QLatin1Char('('));

    if (left < 0) { // No parenthesis, so assume it is a normal encoding name
        return descriptiveName.trimmed();
    }

    QString name(descriptiveName.mid(left + 1));

    const int right = name.lastIndexOf(QLatin1Char(')'));

    if (right < 0) {
        return name;
    }

    return name.left(right).trimmed();
}

QStringList KCharsets::descriptiveEncodingNames() const
{
    QStringList encodings;
    for (const int *p = language_for_encoding_indices; *p != -1; p += 2) {
        const QString name = QString::fromUtf8(language_for_encoding_string + p[0]);
        const QString description = tr(language_for_encoding_string + p[1], "@item Text character set");
        encodings.append(tr("%1 ( %2 )", "@item Text encoding: %1 character set, %2 encoding").arg(description, name));
    }
    encodings.sort();
    return encodings;
}

QList<QStringList> KCharsets::encodingsByScript() const
{
    if (!d->encodingsByScript.isEmpty()) {
        return d->encodingsByScript;
    }
    int i;
    for (const int *p = language_for_encoding_indices; *p != -1; p += 2) {
        const QString name = QString::fromUtf8(language_for_encoding_string + p[0]);
        const QString description = tr(language_for_encoding_string + p[1], "@item Text character set");

        for (i = 0; i < d->encodingsByScript.size(); ++i) {
            if (d->encodingsByScript.at(i).at(0) == description) {
                d->encodingsByScript[i].append(name);
                break;
            }
        }

        if (i == d->encodingsByScript.size()) {
            d->encodingsByScript.append(QStringList() << description << name);
        }
    }
    return d->encodingsByScript;
}

KCharsets *KCharsets::charsets()
{
    return &globalCharsets()->instance;
}
