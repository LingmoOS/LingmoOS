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

#include "limonp/Logging.hpp"
#include "PreFilter.hpp"
#include <cassert>


namespace cppjieba {

const char* const SPECIAL_SEPARATORS = " \t\n\xEF\xBC\x8C\xE3\x80\x82";

using namespace limonp;

class SegmentBase {
public:
    SegmentBase() {
        XCHECK(ResetSeparators(SPECIAL_SEPARATORS));
    }
    virtual ~SegmentBase() { }

    virtual void Cut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange>& res, bool hmm,
                     size_t max_word_len) const = 0;
    //添加基于sentence的cut方法，减少中间变量的存储与格式转换--jxx20210517
    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<string>& res, bool hmm,
                     size_t max_word_len) const = 0;
    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, unordered_map<string, KeyWord>& res, bool hmm,
                     size_t max_word_len) const = 0;
    //重写CutToStr函数，简化获取vector<string>& words的流程，降低内存占用--jxx20210517
    void CutToStr(const string& sentence, vector<string>& words, bool hmm = true,
                  size_t max_word_len = MAX_WORD_LENGTH) const {
        PreFilter pre_filter(symbols_, sentence);
        words.clear();
        words.reserve(sentence.size() / 2);//todo 参考源码，参数待定
        RuneStrArray::const_iterator null_p;
        WordRange range(null_p, null_p);
        while (pre_filter.Next(range)) {
            CutWithSentence(sentence, range.left, range.right, words, hmm, max_word_len);
        }
    }
    void CutToStr(const string& sentence, WordRange range, vector<string>& words, bool hmm = true,
                  size_t max_word_len = MAX_WORD_LENGTH) const {
        CutWithSentence(sentence, range.left, range.right, words, hmm, max_word_len);
    }
    void CutToStr(const string& sentence, WordRange range, unordered_map<string, KeyWord>& words, bool hmm = true,
                  size_t max_word_len = MAX_WORD_LENGTH) const {
        CutWithSentence(sentence, range.left, range.right, words, hmm, max_word_len);
    }
    void CutToWord(const string& sentence, vector<Word>& words, bool hmm = true,
                   size_t max_word_len = MAX_WORD_LENGTH) const {
        PreFilter pre_filter(symbols_, sentence);
        vector<WordRange> wrs;
        wrs.reserve(sentence.size() / 2);

        while (pre_filter.HasNext()) {
            auto range = pre_filter.Next();
            Cut(range.left, range.right, wrs, hmm, max_word_len);
        }

        words.clear();
        words.reserve(wrs.size());
        GetWordsFromWordRanges(sentence, wrs, words);
        wrs.clear();
        vector<WordRange>().swap(wrs);
    }

    void CutRuneArray(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange>& res,
                      bool hmm = true, size_t max_word_len = MAX_WORD_LENGTH) const {
        Cut(begin, end, res, hmm, max_word_len);
    }

    bool ResetSeparators(const string& s) {
        symbols_.clear();
        RuneStrArray runes;

        if (!DecodeRunesInString(s, runes)) {
            XLOG(ERROR) << "decode " << s << " failed";
            return false;
        }

        for (size_t i = 0; i < runes.size(); i++) {
            if (!symbols_.insert(runes[i].rune).second) {
                XLOG(ERROR) << s.substr(runes[i].offset, runes[i].len) << " already exists";
                return false;
            }
        }

        return true;
    }
protected:
    unordered_set<Rune> symbols_;
}; // class SegmentBase

} // cppjieba

