/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef Traditional2Simplified_H
#define Traditional2Simplified_H

#include <QtCore/qglobal.h>
#include <string>
#define TRADITIONAL_CHINESE_SIMPLIFIED_EXPORT Q_DECL_IMPORT

using namespace std;

class Traditional2SimplifiedPrivate;
class TRADITIONAL_CHINESE_SIMPLIFIED_EXPORT Traditional2Simplified
{
public:
    static Traditional2Simplified * getInstance();

public:
    /**
     * @brief Traditional2Simplified::isMultiTone 判断是否为繁体字，是则返回true
     * @param oneWord 要判断的字
     * @return bool 不是返回false
     */
    bool isTraditional(string &oneWord);

    /**
     * @brief Traditional2Simplified::getResults 转换某个字/词/句的繁体字
     * @param words 要转换为简体中文的字/词/句
     * @return words 的简体中文结果
     */
    string getResults(string words);

protected:
    Traditional2Simplified();
    ~Traditional2Simplified();
    Traditional2Simplified(const Traditional2Simplified&) = delete;
    Traditional2Simplified& operator =(const Traditional2Simplified&) = delete;
private:
    static Traditional2Simplified *g_Traditional2SimplifiedManager;
    Traditional2SimplifiedPrivate *d = nullptr;
};

#endif // PINYINMANAGER_H
