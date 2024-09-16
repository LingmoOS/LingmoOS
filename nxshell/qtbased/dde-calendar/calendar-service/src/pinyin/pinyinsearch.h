// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PINYINSEARCH_H
#define PINYINSEARCH_H

#include <QVector>
#include <QMap>

class pinyinsearch
{
public:
    static pinyinsearch *getPinPinSearch();
    /* 判断字符串是否只可以进行拼音查询 */
    static bool CanQueryByPinyin(QString str);
    /* 创建拼音字符串 */
    static QString CreatePinyin(const QString &zh);
    //初始化字典
    static void initDict();
    /* 构造拼音查询表达式 */
    QString CreatePinyinQuery(QString pinyin) const;
    /* 构造拼音查询正则表达式 */
    QString CreatePinyinRegexp(QString pinyin) const;
    /* 判断汉字和拼音是否匹配 */
    bool PinyinMatch(const QString &zh, const QString &py) const;

private:
    /* 初始化合法拼音表 */
    pinyinsearch();
    //获取汉字对应的拼音，不带音调
    static QList<QStringList> Pinyin(QString str);
    //找到汉字对应的拼音
    static QStringList SinglePinyin(QString index);
    //去掉拼音中的音调
    static QStringList RemoveYin(QStringList pinyin);
    /* 单个合法拼音的最大长度 */
    int singlePinyinMaxLength = 0;
    /* 合法单拼音表 */
    QMap<QString, bool> validPinyinMap {};
    static pinyinsearch *m_pinyinsearch;
    //拼音字典
    static QMap<int, QString> pinyinDictVector;
};

#endif
