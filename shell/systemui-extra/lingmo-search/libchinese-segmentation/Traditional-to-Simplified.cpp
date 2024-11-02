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

#include <mutex>
#include <cctype>
#include "Traditional-to-Simplified.h"
#include "Traditional-to-Simplified-private.h"
#include "cppjieba/Unicode.hpp"

Traditional2Simplified * Traditional2Simplified::g_Traditional2SimplifiedManager = nullptr;
std::once_flag g_Traditional2SimplifiedSingleFlag;

string Traditional2SimplifiedPrivate::getResults(string words)
{
    string results;
    if (words.empty()) {
        return words;
    } else if (cppjieba::IsSingleWord(words)) {//单个字符
        results = m_Traditional2SimplifiedTrie.Find(words);
        if (results.empty()) {
            results = words;//原数据返回
        }
    } else {//多个字符
        string oneWord;
        string data;
        cppjieba::RuneStrArray runeArray;
        cppjieba::DecodeRunesInString(words, runeArray);
        for (auto i = runeArray.begin(); i != runeArray.end(); ++i) {
            oneWord = cppjieba::GetStringFromRunes(words, i, i);
            data = m_Traditional2SimplifiedTrie.Find(oneWord);
            if (data.empty()) {//单字无结果
                results.append(oneWord);
            } else {
                results.append(data);
            }
        }
    }
    return results;
}

Traditional2SimplifiedPrivate::Traditional2SimplifiedPrivate(Traditional2Simplified *parent) : q(parent)
{
}

Traditional2SimplifiedPrivate::~Traditional2SimplifiedPrivate()
{
}

Traditional2Simplified * Traditional2Simplified::getInstance()
{
    call_once(g_Traditional2SimplifiedSingleFlag, []() {
        g_Traditional2SimplifiedManager = new Traditional2Simplified;
    });
    return g_Traditional2SimplifiedManager;
}

bool Traditional2Simplified::isTraditional(string &oneWord)
{
    return d->isTraditional(oneWord);
}

string Traditional2Simplified::getResults(string words)
{
    return d->getResults(words);
}

Traditional2Simplified::Traditional2Simplified() : d(new Traditional2SimplifiedPrivate)
{
}
