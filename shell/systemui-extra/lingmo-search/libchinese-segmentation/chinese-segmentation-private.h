/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 */
#ifndef CHINESESEGMENTATIONPRIVATE_H
#define CHINESESEGMENTATIONPRIVATE_H

#include "chinese-segmentation.h"
#include "cppjieba/Jieba.hpp"
#include "cppjieba/KeywordExtractor.hpp"

class ChineseSegmentationPrivate
{
public:
    explicit ChineseSegmentationPrivate(ChineseSegmentation *parent = nullptr);
    ~ChineseSegmentationPrivate();
    vector<KeyWord> callSegment(const string& sentence);
    vector<KeyWord> callSegment(QString& sentence);

    vector<string> callMixSegmentCutStr(const string& sentence);
    vector<Word> callMixSegmentCutWord(const string& sentence);
    string lookUpTagOfWord(const string& word);
    vector<pair<string, string>> getTagOfWordsInSentence(const string &sentence);

    vector<Word> callFullSegment(const string& sentence);

    vector<Word> callQuerySegment(const string& sentence);

    vector<Word> callHMMSegment(const string& sentence);

    vector<Word> callMPSegment(const string& sentence);

private:
    cppjieba::Jieba *m_jieba;
    ChineseSegmentation *q = nullptr;
};

#endif // CHINESESEGMENTATIONPRIVATE_H
