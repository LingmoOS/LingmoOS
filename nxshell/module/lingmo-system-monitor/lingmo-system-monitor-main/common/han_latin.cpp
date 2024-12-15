// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "han_latin.h"
#include "ddlog.h"
#include <QDebug>
#include <QString>
#include <QStringList>

#include "unicode/parseerr.h"
#include "unicode/translit.h"
#include "unicode/utypes.h"

#include <memory>

#define TRANSLITERATION_HAN_LATIN "Han-Latin"
#define TRANSLITERATION_LATIN_ASCII "Latin-ASCII"

using namespace std;
using namespace icu;
using namespace DDLog;
namespace util {
namespace common {

// format icu error
static QString parseError(const QString &words, UErrorCode &ec, const UParseError &pe)
{
    QString errbuf {};

    std::string pre {}, post {};
#undef u_errorName
#define UERRORNAME U_DEF2_ICU_ENTRY_POINT_RENAME(u_errorName, U_ICU_VERSION_SUFFIX)
    errbuf = QString(
                     "Error[%1]: %2 convert %3 to PINYIN failed. [line:%4 offset:%5 "
                     "preContext: %6 postContext: %7]")
                     .arg(ec)
                     .arg(UERRORNAME(ec))
                     .arg(words)
                     .arg(pe.line)
                     .arg(pe.offset)
                     .arg(QString::fromStdString(UnicodeString(pe.preContext).toUTF8String(pre)))
                     .arg(QString::fromStdString(UnicodeString(pe.postContext).toUTF8String(post)));

    return errbuf;
}

QString convHanToLatin(const QString &words)
{
    QString result {};
    UErrorCode ec = U_ZERO_ERROR;
    UParseError pe {};
    unique_ptr<Transliterator> tr(Transliterator::createInstance(
            TRANSLITERATION_HAN_LATIN, UTransDirection::UTRANS_FORWARD, pe, ec));

    UnicodeString ubuf = UnicodeString::fromUTF8(StringPiece(words.toStdString()));
    // from hanzi to latin
    tr->transliterate(ubuf);

    if (U_FAILURE(ec)) {
        qCDebug(app) << parseError(words, ec, pe);
        result = words;
    } else {
        ec = U_ZERO_ERROR;
        pe = {};
        unique_ptr<Transliterator> tr2(Transliterator::createInstance(
                TRANSLITERATION_LATIN_ASCII, UTransDirection::UTRANS_FORWARD, pe, ec));
        // from latin to ascii (pinyin)
        tr2->transliterate(ubuf);

        if (U_FAILURE(ec)) {
            qCDebug(app) << parseError(words, ec, pe);
            result = words;
        } else {
            std::string buffer;
            result = QString::fromStdString(ubuf.toUTF8String(buffer));
        }
    }

    return result;
}

}   // namespace common
}   // namespace util
