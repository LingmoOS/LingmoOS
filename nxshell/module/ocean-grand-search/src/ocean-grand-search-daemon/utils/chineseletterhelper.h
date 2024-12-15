// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHINESELETTERHELPER_H
#define CHINESELETTERHELPER_H

#include <QHash>
#include <QObject>
#include <QReadWriteLock>

#define Ch2PyIns GrandSearch::ChineseLetterHelper::instance()

namespace GrandSearch {

class ChineseLetterHelper
{
public:
    static ChineseLetterHelper *instance();
    bool convertChinese2Pinyin(const QString &inStr, QString &outFirstPy, QString &outFullPy);
protected:
    ChineseLetterHelper();
    bool chinese2Pinyin(const QString &words, QString &result);
private:
    void initDict();
    volatile bool m_inited = false;
    QHash<uint, QString> m_dict;
    QReadWriteLock m_lock;
};

}

#endif // CHINESELETTERHELPER_H
