// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chineseletterhelper.h"

#include <QFile>
#include <QTextStream>

using namespace GrandSearch;

class ChineseLetterHelperGlobal : public ChineseLetterHelper {};
Q_GLOBAL_STATIC(ChineseLetterHelperGlobal, chineseLetterHelperGlobal)

ChineseLetterHelper *ChineseLetterHelper::instance()
{
    return chineseLetterHelperGlobal;
}

ChineseLetterHelper::ChineseLetterHelper()
{

}

bool ChineseLetterHelper::chinese2Pinyin(const QString &words, QString &result)
{
    QReadLocker lk(&m_lock);

    int ok = false;
    for (int i = 0; i < words.length(); ++i) {
        const uint key = words.at(i).unicode();
        auto found = m_dict.find(key);

        if (found != m_dict.end()) {
            result.append(found.value());
            ok = true;
        } else {
            result.append(words.at(i));
        }
    }

    return ok;
}

void ChineseLetterHelper::initDict()
{
    // fast check
    if (m_inited)
        return;

    QWriteLocker lk(&m_lock);

    // check again, m_inited may be setted to true in other thread.
    if (m_inited)
        return;

    m_inited = true;

    const QString dictPath = ":/misc/grand-search-daemon/pinyin.dict";
    const int maxWord = 25333;
    QHash<uint, QString> dict;
    dict.reserve(maxWord);

    QFile file(dictPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray content = file.readAll();
    file.close();

    QTextStream stream(&content, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList items = line.split(QChar(':'));

        if (items.size() == 2) {
            dict.insert(static_cast<uint>(items[0].toInt(nullptr, 16)), items[1]);
        }
    }

    m_dict = dict;
}

bool ChineseLetterHelper::convertChinese2Pinyin(const QString &inStr, QString &outFirstPy, QString &outFullPy)
{
    if (inStr.isEmpty())
        return false;

    initDict();

    bool ok = false;
    for (int i = 0; i < inStr.size(); ++i) {
        const QString &cur = inStr.at(i);
        QString py;
        if (chinese2Pinyin(cur, py)) {
            ok = true;

            if (py.isEmpty())
                continue;

            outFirstPy.append(py.at(0));
            outFullPy.append(py);
        } else {
            outFirstPy.append(cur);
            outFullPy.append(cur);
        }
    }

    return ok;
}

