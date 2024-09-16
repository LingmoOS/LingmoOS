// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dpinyin.h"

#include <QFile>
#include <QSet>
#include <QTextStream>
#include <QMap>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

static QHash<uint, QString> dict = {};
const char kDictFile[] = ":/dpinyin/resources/dpinyin.dict";

static void InitDict() {
    if (!dict.isEmpty()) {
        return;
    }

    dict.reserve(25333);

    QFile file(kDictFile);

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray content = file.readAll();

    file.close();

    QTextStream stream(&content, QIODevice::ReadOnly);

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        // comment
        if (line.startsWith("#"))
            continue;
        const QStringList items = line.left(line.indexOf("#")).split(QChar(':'));

        if (items.size() == 2) {
            dict.insert(items[0].toUInt(nullptr, 16), items[1].trimmed());
        }
    }
}

static void initToneTable(QMap<QChar, QString> &toneTable)
{
    if (toneTable.size() > 0)
        return;

    const QString ts = "aāáǎà,oōóǒò,eēéěè,iīíǐì,uūúǔù,vǖǘǚǜ";

    for (const QString &s : ts.split(",")) {
        for (int i = 1; i < s.length(); ++i) {
            toneTable.insert(s.at(i), QString("%1%2").arg(s.at(0)).arg(i));
        }
    }
}

static QString toned(const QString &str, ToneStyle ts)
{
    // TS_Tone is default
    if (ts == TS_Tone)
        return str;

    static QMap<QChar, QString> toneTable; // {ā, a1}
    initToneTable(toneTable);

    QString newStr = str;
    QString cv;
    for (QChar c : str) {
        if (!toneTable.contains(c))
            continue;

        cv = toneTable.value(c);
        switch (ts) {
        case TS_NoneTone:
            newStr.replace(c, cv.left(1));
            break;
        case TS_ToneNum:
            newStr.replace(c, cv);
            break;
        default:
            break;
        }
    }
    return newStr;
}

static QStringList toned(const QStringList &words, ToneStyle ts)
{
    QStringList tonedWords;
    for (auto str : words)
        tonedWords << toned(str, ts);

    return tonedWords;
}

static QStringList permutations(const QStringList &list1, const QStringList &list2)
{
    QStringList ret;
    for (const QString &str1 : list1)
        for (const QString &str2 : list2)
            ret << str1 + str2;

    return ret;
}

static QStringList permutations(const QList<QStringList> &pyList)
{
    QStringList result;

    if (pyList.size() <= 1)
        return pyList.value(0, result);

    result = permutations(pyList.value(0), pyList.value(1));

    for (int i = 2; i < pyList.size(); ++i) {
        result = permutations(result, pyList.value(i));

        // 限制返回的大小，
        if (result.size() > 0xFFFF) {
            qWarning() << "Warning: Too many combinations have exceeded the limit\n";
            break;
        }
    }

    return result;
}

static QStringList deduplication(const QStringList &list)
{
    QStringList result;
    for (const QString &item : list)
        if (!result.contains(item))
            result.append(item);

    return result;
}

/*!
  \fn QString Dtk::Core::Chinese2Pinyin(const QString &words)
  \brief Convert Chinese characters to Pinyin
  \note this function not support polyphonic characters support.
  \return pinyin of the words
  \sa Dtk::Core::pinyin
 */
QString Chinese2Pinyin(const QString &words)
{
    QStringList res = pinyin(words, TS_ToneNum);

    return res.value(0);
}

/*!
 * @~english
  \enum Dtk::Core::ToneStyle
   pinyin tone style

  \value TS_NoneTone pinyin without tone

  \value TS_Tone pinyin with tone, default style in dictory file

  \value TS_ToneNum pinyin tone number

 */

/*!
  \fn QStringList Dtk::Core::pinyin(const QString &words, ToneStyle ts, bool *ok)
  \brief Convert Chinese characters to Pinyin with polyphonic characters support.

  \return pinyin list of the words
 */
QStringList pinyin(const QString &words, ToneStyle ts, bool *ok)
{
    if (words.length() < 1)
        return QStringList();

    InitDict();

    if (ok)
        *ok = true;
    QList<QStringList> pyList;
    for (int i = 0; i < words.length(); ++i) {
        const uint key = words.at(i).unicode();
        auto find_result = dict.find(key);

        if (find_result != dict.end()) {
            const QString &ret = find_result.value();
            pyList << toned(ret.split(","), ts);

        } else {
            pyList << QStringList(words.at(i));
            // 部分字没有在词典中找到，使用字本身， ok 可以判断结果
            if (ok)
                *ok = false;
        }
    }

    return deduplication(permutations(pyList));
}

/*!
  \fn QStringList Dtk::Core::firstLetters(const QString &words)
  \brief Convert Chinese characters to Pinyin firstLetters list
  \brief with polyphonic characters support.

  \return pinyin first letters list of the words
 */
QStringList firstLetters(const QString &words)
{
    QList<QStringList> result;
    bool ok = false;
    for (const QChar &w : words) {
        QStringList pys = pinyin(w, TS_Tone, &ok);
        if (!ok) {
            result << QStringList(w);
            continue;
        }

        for (QString &py : pys)
            py = py.left(1);

        result << pys;
    }

    return deduplication(permutations(result));
}

DCORE_END_NAMESPACE
