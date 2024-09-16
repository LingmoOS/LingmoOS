// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "languagetranformation.h"

#include <QFile>
#include <QDebug>
#include <QtConcurrent>

LanguageTransformation *LanguageTransformation::m_instance = Q_NULLPTR;

LanguageTransformation::LanguageTransformation(QObject *parent)
    : QObject (parent)
{
    readConfigFile();
}

LanguageTransformation *LanguageTransformation::instance()
{
    if (!m_instance)
        m_instance = new LanguageTransformation;

    return m_instance;
}

/** 中文转拼音
 * @brief LanguageTransformation::zhToPinYin
 * @param chinese 汉语文字
 * @return 汉语的全拼音，以空格隔开
 */
QString LanguageTransformation::zhToPinYin(const QString &chinese)
{
    QStringList list;
    for (int i = 0; i < chinese.length(); ++i) {
        int unicode = QString::number(chinese.at(i).unicode(), 10).toInt();
        if (unicode >= 0x4E00 && unicode <= 0x9FA5) {
            // m_pinyinStrList是按照UNICODE每个汉字对应的拼音数组
            if ((unicode - 0x4E00) < m_pinyinStrList.size())
                list.append(m_pinyinStrList.at(unicode - 0x4E00));
        } else {
            list.append(chinese.at(i));
        }
    }

    return list.join("");
}

/** 中文转简拼
 * @brief LanguageTransformation::zhToJianPin
 * @param chinese 汉语文字
 * @return 汉语的简拼
 */
QString LanguageTransformation::zhToJianPin(const QString &chinese)
{
    if(chinese.isEmpty())
        return chinese;

    QString strChineseFirstPY = m_jianpingStrList.join("");

    QString jianPinStr;
    int index = 0;
    for(int i = 0; i < chinese.length(); i++) {
        //若是字母或数字则直接输出
        ushort vChar = chinese.at(i).unicode() ;
        if((vChar >= 'a' && vChar <= 'z' ) || (vChar >= 'A' && vChar <= 'Z')) {
            jianPinStr.append(chinese.at(i).toUpper());
        }

        if((vChar >= '0' && vChar <= '9')) {
            jianPinStr.append(chinese.at(i));
        } else {
            index = (int)vChar - 19968;
            if(index >= 0 && index < strChineseFirstPY.length()) {
                jianPinStr.append(strChineseFirstPY.at(index));
            }
        }
    }

    return jianPinStr;
}

void LanguageTransformation::readConfigFile()
{
    QtConcurrent::run([ this ](){
        //从配置文件读取拼音库
        QFile pinYinFile(":/language/pinyin.txt");
        if (!pinYinFile.open(QFile::ReadOnly | QFile::Text)) {
            qWarning() << "pinyin.txt read error!!";
            return;
        }

        QString pinyinStr = pinYinFile.readAll();
        m_pinyinStrList = pinyinStr.split(" ");
        pinYinFile.close();

        // 加载简拼配置库
        QFile jianpinFile(":/language/jianpin.txt");
        if (!jianpinFile.open(QFile::ReadOnly | QFile::Text)) {
            qWarning() << "jianpin.txt read error!!";
            return;
        }

        QString jianpinStr = jianpinFile.readAll();
        m_jianpingStrList.append(jianpinStr);
//        qInfo() << "m_jianpingStrList: " << m_jianpingStrList;
    });
}
