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
#include "SegmentBase.hpp"
#include "Unicode.hpp"

namespace cppjieba {
class FullSegment: public SegmentBase {
public:
    FullSegment(const DictTrie* dictTrie)
        : dictTrie_(dictTrie) {
        assert(dictTrie_);
    }
    ~FullSegment() { }

    virtual void Cut(RuneStrArray::const_iterator begin,
                     RuneStrArray::const_iterator end,
                     vector<WordRange>& res, bool, size_t) const override {
        assert(dictTrie_);
        vector<struct DatDag> dags;
        dictTrie_->FindDatDag(begin, end, dags);
        size_t max_word_end_pos = 0;

        for (size_t i = 0; i < dags.size(); i++) {
            for (const auto & kv : dags[i].nexts) {
                const size_t nextoffset = kv.first - 1;
                assert(nextoffset < dags.size());
                const auto wordLen = nextoffset - i + 1;
                const bool is_not_covered_single_word = ((dags[i].nexts.size() == 1) && (max_word_end_pos <= i));
                const bool is_oov = (nullptr == kv.second); //Out-of-Vocabulary

                if ((is_not_covered_single_word) || ((not is_oov) && (wordLen >= 2))) {
                    WordRange wr(begin + i, begin + nextoffset);
                    res.push_back(wr);
                }

                max_word_end_pos = max(max_word_end_pos, nextoffset + 1);
            }
        }
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
private:
    const DictTrie* dictTrie_;
};
}

