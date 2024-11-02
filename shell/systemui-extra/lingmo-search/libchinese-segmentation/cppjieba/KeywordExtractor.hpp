/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2013 Yanyi Wu
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
*/
#pragma once

#include <cmath>
#include "MixSegment.hpp"
//#include "IdfTrie.hpp"
#include "idf-trie/idf-trie.h"

namespace cppjieba {

using namespace limonp;
using namespace std;

/*utf8*/
class KeywordExtractor {
public:

    KeywordExtractor(const DictTrie* dictTrie,
                     const HMMModel* model,
                     const string& idfPath,
                     const string& dat_cache_path,
                     const string& stopWordPath)
        : segment_(dictTrie, model, stopWordPath),
          idf_trie_(idfPath, dat_cache_path){
    }
    ~KeywordExtractor() {
    }

    void Extract(const string& sentence, vector<string>& keywords, size_t topN) const {
        vector<KeyWord> topWords;
        Extract(sentence, topWords, topN);

        for (size_t i = 0; i < topWords.size(); i++) {
            keywords.push_back(topWords[i].word);
        }
    }

    void Extract(const string& sentence, vector<pair<string, double> >& keywords, size_t topN) const {
        vector<KeyWord> topWords;
        Extract(sentence, topWords, topN);

        for (size_t i = 0; i < topWords.size(); i++) {
            keywords.push_back(pair<string, double>(topWords[i].word, topWords[i].weight));
        }
    }

    void Extract(const string& sentence, vector<KeyWord>& keywords, size_t topN) const {

        unordered_map<string, KeyWord> wordmap;//插入字符串与Word的map，相同string统计词频叠加权重
        PreFilter pre_filter(symbols_, sentence);
        RuneStrArray::const_iterator null_p;
        WordRange range(null_p, null_p);
        bool isNull(false);
        while (pre_filter.Next(range, isNull)) {
            if (isNull) {
                continue;
            }
            segment_.CutToStr(sentence, range,  wordmap);
        }

        keywords.clear();
        keywords.reserve(wordmap.size());

        for (unordered_map<string, KeyWord>::iterator itr = wordmap.begin(); itr != wordmap.end(); ++itr) {
            double idf = idf_trie_.Find(itr->first);
            if (-1 != idf) {//IDF词典查找
                itr->second.weight *= idf;
            } else {
                itr->second.weight *= idf_trie_.GetIdfAverage();
            }

            itr->second.word = itr->first;
            keywords.push_back(itr->second);
        }

        topN = min(topN, keywords.size());
        partial_sort(keywords.begin(), keywords.begin() + topN, keywords.end(), Compare);
        keywords.resize(topN);
    }
private:

    static bool Compare(const KeyWord& lhs, const KeyWord& rhs) {
        return lhs.weight > rhs.weight;
    }

    MixSegment segment_;
    IdfTrie idf_trie_;


    unordered_set<Rune> symbols_;
}; // class KeywordExtractor

inline ostream& operator << (ostream& os, const KeyWord& word) {
    return os << "{\"word\": \"" << word.word << "\", \"offset\": " << word.offsets << ", \"weight\": " << word.weight <<
           "}";
}

} // namespace cppjieba



