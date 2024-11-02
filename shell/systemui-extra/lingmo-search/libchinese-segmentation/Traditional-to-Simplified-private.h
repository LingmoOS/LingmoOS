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

#ifndef Traditional2SimplifiedPRIVATE_H
#define Traditional2SimplifiedPRIVATE_H

#include <QtCore/qglobal.h>
#include <QHash>
#include "Traditional-to-Simplified.h"
#include "Traditional2Simplified_trie.h"

using namespace std;

class TRADITIONAL_CHINESE_SIMPLIFIED_EXPORT Traditional2SimplifiedPrivate
{
public:
    Traditional2SimplifiedPrivate(Traditional2Simplified *parent = nullptr);
    ~Traditional2SimplifiedPrivate();

public:
    bool isTraditional(string &word) {return m_Traditional2SimplifiedTrie.IsTraditional(word);}

    string getResults(string words);

private:

    Traditional2Simplified *q = nullptr;
    Traditional2SimplifiedTrie m_Traditional2SimplifiedTrie;
};
#endif // Traditional2SimplifiedPRIVATE_H
