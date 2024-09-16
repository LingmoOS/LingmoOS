// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPINYIN_H
#define DPINYIN_H

#include <dtkcore_global.h>

#include <QHash>

DCORE_BEGIN_NAMESPACE

// without polyphonic support
QString LIBDTKCORESHARED_EXPORT Chinese2Pinyin(const QString& words);

//!< @~english enum ToneStyle - pinyin tone style
enum ToneStyle {
    TS_NoneTone,  /*!< @~english pinyin without tone */
    TS_Tone,      /*!< @~english pinyin tone, default style in dictory file*/
    TS_ToneNum,   /*!< @~english pinyin tone number */
};

// support polyphonic
QStringList LIBDTKCORESHARED_EXPORT pinyin(const QString& words, ToneStyle ts = TS_Tone, bool *ok = nullptr);

// support polyphonic
QStringList LIBDTKCORESHARED_EXPORT firstLetters(const QString& words);

DCORE_END_NAMESPACE

#endif // DPINYIN_H
