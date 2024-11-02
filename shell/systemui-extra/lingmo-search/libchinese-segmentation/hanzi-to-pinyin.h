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

#ifndef HANZITOPINYIN_H
#define HANZITOPINYIN_H

#include <QtCore/qglobal.h>
#include <QStringList>
#include "pinyin4cpp-common.h"
#define PINYINMANAGER_EXPORT Q_DECL_IMPORT

using namespace std;

class HanZiToPinYinPrivate;
class PINYINMANAGER_EXPORT HanZiToPinYin
{
public:
    static HanZiToPinYin * getInstance();

public:
    /**
     * @brief HanZiToPinYin::isMultiTone 判断是否为多音字/词/句
     * @param word 要判断的字/词/句
     * @return bool 不是返回false
     */
    bool isMultiTone(string &word);
    bool isMultiTone(string &&word);
    bool isMultiTone(const string &word);
    bool isMultiTone(const string &&word);

    /**
     * @brief HanZiToPinYin::contains 查询某个字/词/句是否有拼音（是否在数据库包含）
     * @param word 要查询的字/词/句
     * @return bool 数据库不包含返回false
     */
    bool contains(string &word);

    /**
     * @brief HanZiToPinYin::getResults 获取某个字/词/句的拼音
     * @param word 要获取拼音的字/词/句
     * @param results word的拼音列表（有可能多音字），每次调用results会被清空
     * @return int 获取到返回0，否则返回-1
     */
    int getResults(string word, QStringList &results);

    /**
     * @brief setConfig 设置HanZiToPinYin的各项功能，详见pinyin4cpp-common.h
     * @param dataStyle 返回数据风格，默认defult
     * @param segType 是否启用分词，默认启用
     * @param polyphoneType 是否启用多音字，默认不启用
     * @param processType 无拼音数据处理模式，默认defult
     */
    void setConfig(PinyinDataStyle dataStyle,SegType segType,PolyphoneType polyphoneType,ExDataProcessType processType);

protected:
    HanZiToPinYin();
    ~HanZiToPinYin();
    HanZiToPinYin(const HanZiToPinYin&) = delete;
    HanZiToPinYin& operator =(const HanZiToPinYin&) = delete;
private:
    static HanZiToPinYin *g_pinYinManager;
    HanZiToPinYinPrivate *d = nullptr;
};

#endif // PINYINMANAGER_H
