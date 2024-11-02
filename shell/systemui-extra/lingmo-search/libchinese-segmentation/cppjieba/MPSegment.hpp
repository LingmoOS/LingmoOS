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

#include <algorithm>
#include <set>
#include <cassert>
#include "limonp/Logging.hpp"
#include "segment-trie/segment-trie.h"
//#include "DictTrie.hpp"
#include "SegmentTagged.hpp"
#include "PosTagger.hpp"

namespace cppjieba {

class MPSegment: public SegmentTagged {
public:
    MPSegment(const DictTrie* dictTrie)
        : dictTrie_(dictTrie) {
        assert(dictTrie_);
    }
    ~MPSegment() { }

    virtual void Cut(RuneStrArray::const_iterator begin,
                     RuneStrArray::const_iterator end,
                     vector<WordRange>& words,
                     bool, size_t max_word_len) const override {
        dictTrie_->FindWordRange(begin, end, words, max_word_len);
    }

    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<string>& res, bool hmm,
                     size_t) const override {
        std::ignore = s;
        std::ignore = begin;
        std::ignore = end;
        std::ignore = res;
        std::ignore = hmm;
    }
    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, unordered_map<string, KeyWord>& res, bool hmm,
                     size_t) const override {
        std::ignore = s;
        std::ignore = begin;
        std::ignore = end;
        std::ignore = res;
        std::ignore = hmm;
    }
    const DictTrie* GetDictTrie() const override {
        return dictTrie_;
    }

    bool Tag(const string& src, vector<pair<string, string> >& res) const override {
        return tagger_.Tag(src, res, *this);
    }

    bool IsUserDictSingleChineseWord(const Rune& value) const {
        return dictTrie_->IsUserDictSingleChineseWord(value);
    }
private:
/*
    void CalcDP(vector<DatDag>& dags) const {
        double val(0);
        for (auto rit = dags.rbegin(); rit != dags.rend(); rit++) {
            rit->max_next = -1;
            rit->max_weight = MIN_DOUBLE;

            for (const auto & it : rit->nexts) {
                const auto nextPos = it.first;
                val = dictTrie_->GetMinWeight();

                if (nullptr != it.second) {
                    val = it.second->weight;
                }

                if (nextPos  < dags.size()) {
                    val += dags[nextPos].max_weight;
                }

                if ((nextPos <= dags.size()) && (val > rit->max_weight)) {
                    rit->max_weight = val;
                    rit->max_next = nextPos;
                }
            }
        }
    }
*/
/*  倒叙方式重写CalcDP函数，初步测试未发现问题*/
/*
    void CalcDP(vector<DatDag>& dags) const {
        double val(0);
        size_t size = dags.size();

        for (size_t i = 0; i < size; i++) {
            dags[size - 1 - i].max_next = -1;
            dags[size - 1 - i].max_weight = MIN_DOUBLE;

            for (const auto & it : dags[size - 1 - i].nexts) {
                const auto nextPos = it.first;
                if (nullptr != it.second) {
                    val = it.second->weight;
                }

                if (nextPos  < dags.size()) {
                    val += dags[nextPos].max_weight;
                }

                if ((nextPos <= dags.size()) && (val > dags[size - 1 - i].max_weight)) {
                    dags[size - 1 - i].max_weight = val;
                    dags[size - 1 - i].max_next = nextPos;
                }
            }
        }
    }

    void CutByDag(RuneStrArray::const_iterator begin,
                  RuneStrArray::const_iterator,
                  const vector<DatDag>& dags,
                  vector<WordRange>& words) const {

        for (size_t i = 0; i < dags.size();) {
            const auto next = dags[i].max_next;
            assert(next > i);
            assert(next <= dags.size());
            WordRange wr(begin + i, begin + next - 1);
            words.push_back(wr);
            i = next;
        }
    }
*///相关功能已集成到Find函数中
    const DictTrie* dictTrie_;
    PosTagger tagger_;

}; // class MPSegment

} // namespace cppjieba

