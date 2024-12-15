// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pinyinsearch.h"
#include "pinyindict.h"
#include "commondef.h"
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QDebug>

pinyinsearch *pinyinsearch::m_pinyinsearch = nullptr;
QMap<int, QString> pinyinsearch::pinyinDictVector {};

const QString kDictFile = ":/pinyin.dict";

/**
 * @brief pinyinsearch::pinyinsearch 构造函数，进行初始化
 */
pinyinsearch::pinyinsearch()
{
    for (int i = 0; i < validPinyinList.size(); i++) {
        QString key = validPinyinList[i];
        validPinyinMap[key] = true;
        //拼音最大长度
        if (key.size() > singlePinyinMaxLength)
            singlePinyinMaxLength = key.size();
    }
}

pinyinsearch *pinyinsearch::getPinPinSearch()
{
    if (m_pinyinsearch == nullptr) {
        //new pinyinsearch
        m_pinyinsearch = new pinyinsearch();
        //获取拼音字典
        initDict();
    }
    return m_pinyinsearch;
}

/**
 * @brief pinyinsearch::canQueryByPinyin 拼音是否与正则表达式匹配
 * @param str 拼音字符串
 * @return bool值
 */
bool pinyinsearch::CanQueryByPinyin(QString str)
{
    QRegExp regexp("^[a-zA-Z]+$");
    return regexp.exactMatch(str);
}

/**
 * @brief pinyinsearch::createPinyin 创建拼音字符串
 * @param zh 汉字字符串
 * @return 拼音字符串
 */
QString pinyinsearch::CreatePinyin(const QString &zh)
{
    //返回中文对应的拼音，汉字个数对应QList长度，每个汉字拥有的拼音个数对应QStringList长度
    QList<QStringList> pyList = Pinyin(zh);
    QString pinyinStr;
    for (int i = 0; i < pyList.count(); i++) {
        //如果一个汉字有多个拼音，把“|”放在中间，以示区分
        QString pyStr = pyList.at(i).join("|");
        //将每个汉字的拼音放在[]中
        if (!pyStr.isEmpty())
            pinyinStr += "[" + pyStr + "]";
    }
    return pinyinStr;
}

void pinyinsearch::initDict()
{
    if (!pinyinDictVector.isEmpty()) {
        return;
    }

    QFile file(kDictFile);

    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ServiceLogger) << "open dictFile error :" << file.error();
        return;
    }

    QByteArray content = file.readAll();
    file.close();
    QTextStream stream(&content, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList items = line.split(QChar(':'));

        if (items.size() == 2) {
            pinyinDictVector.insert(items[0].toInt(nullptr, 16), items[1]);
        }
    }
}

/**
 * @brief pinyinsearch::createPinyinQuery 构造拼音查询表达式
 * @param pinyin 拼音
 * @return 按照汉字划分好的拼音字符串
 */
QString pinyinsearch::CreatePinyinQuery(QString pinyin) const
{
    QString expr;
    //对传入的拼音进行划分，例如：“nihao”->"[%ni%][%hao%]"
    while (pinyin.size() > 0) {
        //拼音最大长度
        int i = singlePinyinMaxLength;
        if (i > pinyin.size()) {
            i = pinyin.size();
        }
        while (i > 1) {
            //从拼音最大长度，递减截取拼音，是否在字典中
            QString key = pinyin.mid(0, i);
            //如果拼音在字典中，则此拼音代表一个汉字的拼音，break
            if (validPinyinMap[key])
                break;
            i--;
        }
        //一个汉字的拼音
        QString key = pinyin.mid(0, i);
        //对剩下的字符串再次进行解析
        pinyin = pinyin.mid(i, pinyin.size());
        //对每个汉字的拼音进行区分
        expr += "[%" + key + "%]";
    }
    return expr;
}

/**
 * @brief pinyinsearch::createPinyinRegexp 构造拼音查询正则表达式
 * @param pinyin 拼音
 * @return 拼音的正则表达式
 */
QString pinyinsearch::CreatePinyinRegexp(QString pinyin) const
{
    QString expr;
    while (pinyin.size() > 0) {
        int i = singlePinyinMaxLength;
        if (i > pinyin.size()) {
            i = pinyin.size();
        }
        while (i > 1) {
            QString key = pinyin.mid(0, i);
            if (validPinyinMap[key]) {
                break;
            }
            i--;
        }
        QString key = pinyin.mid(0, i);
        pinyin = pinyin.mid(i, pinyin.size());
        expr += "\\[[a-z\\|]*" + key + "[a-z\\|]*\\]";
    }
    return expr;
}

/**
 * @brief pinyinsearch::pinyinMatch 判断汉字和拼音是否匹配
 * @param zh 汉字
 * @param py 汉字对应的拼音
 * @return bool值
 */
bool pinyinsearch::PinyinMatch(const QString &zh, const QString &py) const
{
    //获取汉字的拼音
    QString zhPinyin = CreatePinyin(zh);
    //获取拼音的正则表达式
    QString expr = CreatePinyinRegexp(py);
    //QRegularExpression类使用正则表达式提供模式匹配。
    QRegularExpression regexp(expr);
    //根据给定的主题字符串匹配正则表达式,返回的QRegularExpressionMatch对象包含匹配的结果。
    QRegularExpressionMatch match = regexp.match(zhPinyin);
    //拼音是否与正则表达式匹配
    return match.hasMatch();
}

/**
 * @brief pinyinsearch::Pinyin 获取汉字的拼音
 * @param str 汉字
 * @return 汉字对应的拼音
 */
QList<QStringList> pinyinsearch::Pinyin(QString str)
{
    QList<QStringList> pys {};
    QStringList py;
    for (int i = 0; i < str.count(); i++) {
        //每个汉字对应的拼音
        py = SinglePinyin(str.at(i));
        if (py.size() > 0) {
            //将去掉音调的拼音添加到Qlist中
            pys.append(RemoveYin(py));
        }
    }
    return pys;
}

/**
 * @brief pinyinsearch::SinglePinyin 通过字典找到汉字对应的拼音
 * @param index 汉字
 * @return 拼音
 */
QStringList pinyinsearch::SinglePinyin(QString index)
{
    //通过汉字的编码找到对应的拼音
    QString value;
    //查找对应的拼音
    const int unicode = index.data()->unicode();

    if (pinyinDictVector.contains(unicode)) {
        value = pinyinDictVector[index.data()->unicode()];
    }
    QStringList pys {};
    pys = value.split(",");
    return pys;
}

/**
 * @brief pinyinsearch::removeYin 去掉拼音中的音调
 * @param pinyin 带有音调的拼音
 * @return 不带音调的拼音
 */
QStringList pinyinsearch::RemoveYin(QStringList pinyin)
{
    QString str;
    QStringList strList;
    //遍历每个拼音
    for (int i = 0; i < pinyin.size(); i++) {
        //遍历拼音中的每个字母，对带有音调的进行处理
        for (int j = 0; j < pinyin.at(i).size(); j++) {
            QString s = pinyin.at(i).at(j);
            //如果有音调转换为不带音调的字母，否则使用原字母
            if (phoneticSymbol.contains(s)) {
                str.append(phoneticSymbol[s][0]);
            } else {
                str.append(s);
            }
        }
        //一个拼音可能有多种音调，所以需要去重
        if (!strList.contains(str))
            strList.append(str);
        str.clear();
    }
    return strList;
}
