/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: jixiaoxu <jixiaoxu@kylinos.cn>
 *
 */

#ifndef HANZITOPINYINPRIVATE_H
#define HANZITOPINYINPRIVATE_H

#include <QtCore/qglobal.h>
#include <QHash>
#include "pinyin4cpp_dictTrie.h"
#include "hanzi-to-pinyin.h"
#include "pinyin4cpp-trie.h"

using namespace std;

static const QHash<QString, QString> PhoneticSymbol = {
    {"ā", "a1"}, {"á", "a2"}, {"ǎ", "a3"}, {"à", "a4"},
    {"ē", "e1"}, {"é", "e2"}, {"ě", "e3"}, {"è", "e4"},
    {"ō", "o1"}, {"ó", "o2"}, {"ǒ", "o3"}, {"ò", "o4"},
    {"ī", "i1"}, {"í", "i2"}, {"ǐ", "i3"}, {"ì", "i4"},
    {"ū", "u1"}, {"ú", "u2"}, {"ǔ", "u3"}, {"ù", "u4"},
    // üe
    {"ü", "v"},
    {"ǖ", "v1"}, {"ǘ", "v2"}, {"ǚ", "v3"}, {"ǜ", "v4"},
    {"ń", "n2"}, {"ň", "n3"}, {"ǹ", "n4"},
    {"m̄", "m1"}, {"ḿ", "m2"}, {"m̀", "m4"},
    {"ê̄", "ê1"}, {"ế", "ê2"}, {"ê̌", "ê3"}, {"ề", "ê4"}
};

#define PINYINMANAGER_EXPORT Q_DECL_IMPORT

class PINYINMANAGER_EXPORT HanZiToPinYinPrivate
{
public:
    HanZiToPinYinPrivate(HanZiToPinYin *parent = nullptr);
    ~HanZiToPinYinPrivate();

public:
    template <typename T>
    bool isMultiTone(T &&t) {return m_pinYinTrie.IsMultiTone(std::forward<T>(t));}

    bool contains(string &word);
    int getResults(string &word, QStringList &results);
    void setConfig(PinyinDataStyle dataStyle, SegType segType, PolyphoneType polyphoneType, ExDataProcessType processType);

private:
    void convertDataStyle(QStringList &results);

    HanZiToPinYin *q = nullptr;
    //Pinyin4cppDictTrie *m_pinYinTrie = nullptr;
    Pinyin4cppTrie m_pinYinTrie;

    SegType m_segType = SegType::Segmentation;
    PolyphoneType m_polyphoneType = PolyphoneType::Disable;
    PinyinDataStyle m_pinyinDataStyle = PinyinDataStyle::Default;
    ExDataProcessType m_exDataProcessType = ExDataProcessType::Default;
};
#endif // HANZITOPINYINPRIVATE_H
