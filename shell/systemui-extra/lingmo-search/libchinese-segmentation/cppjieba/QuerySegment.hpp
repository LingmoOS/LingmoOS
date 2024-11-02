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
#include "SegmentBase.hpp"
#include "FullSegment.hpp"
#include "MixSegment.hpp"
#include "Unicode.hpp"

namespace cppjieba {
class QuerySegment: public SegmentBase {
public:
    QuerySegment(const DictTrie* dictTrie,
                 const HMMModel* model,
                 const string& stopWordPath)
        : mixSeg_(dictTrie, model, stopWordPath), trie_(dictTrie) {
    }
    ~QuerySegment() {
    }

    virtual void Cut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange>& res, bool hmm,
                     size_t) const override {
        //use mix Cut first
        vector<WordRange> mixRes;
        mixSeg_.CutRuneArray(begin, end, mixRes, hmm);

        vector<WordRange> fullRes;

        for (vector<WordRange>::const_iterator mixResItr = mixRes.begin(); mixResItr != mixRes.end(); mixResItr++) {
            if (mixResItr->Length() > 2) {
                for (size_t i = 0; i + 1 < mixResItr->Length(); i++) {
                    string text = EncodeRunesToString(mixResItr->left + i, mixResItr->left + i + 2);

                    if (trie_->Find(text) != nullptr) {
                        WordRange wr(mixResItr->left + i, mixResItr->left + i + 1);
                        res.push_back(wr);
                    }
                }
            }

            if (mixResItr->Length() > 3) {
                for (size_t i = 0; i + 2 < mixResItr->Length(); i++) {
                    string text = EncodeRunesToString(mixResItr->left + i, mixResItr->left + i + 3);

                    if (trie_->Find(text) != nullptr) {
                        WordRange wr(mixResItr->left + i, mixResItr->left + i + 2);
                        res.push_back(wr);
                    }
                }
            }

            res.push_back(*mixResItr);
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
    bool IsAllAscii(const RuneArray& s) const {
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] >= 0x80) {
                return false;
            }
        }

        return true;
    }
    MixSegment mixSeg_;
    const DictTrie* trie_;
}; // QuerySegment

} // namespace cppjieba

