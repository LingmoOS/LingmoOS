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

#include <cassert>
#include "MPSegment.hpp"
#include "HMMSegment.hpp"
#include "limonp/StringUtil.hpp"
#include "PosTagger.hpp"
#define STOP_WORDS_USE_CEDAR_SEGMENT //使用cedar初步测试性能提升3%-5%左右，内存占用降低近不明显
#ifdef STOP_WORDS_USE_CEDAR_SEGMENT
#include "cedar/cedar.h"
#endif

namespace cppjieba {
class MixSegment: public SegmentTagged {
public:
    MixSegment(const DictTrie* dictTrie,
               const HMMModel* model,
               const string& stopWordPath)
        : mpSeg_(dictTrie), hmmSeg_(model) {
        LoadStopWordDict(stopWordPath);
    }
    ~MixSegment() {}

    virtual void Cut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange>& res, bool hmm,
                     size_t) const override {
        if (!hmm) {
            mpSeg_.CutRuneArray(begin, end, res);
            return;
        }

        vector<WordRange> words;
        assert(end >= begin);
        words.reserve(end - begin);
        mpSeg_.CutRuneArray(begin, end, words);

        vector<WordRange> hmmRes;
        hmmRes.reserve(end - begin);

        for (size_t i = 0; i < words.size(); i++) {
            //if mp Get a word, it's ok, put it into result
            if (words[i].left != words[i].right || (words[i].left == words[i].right &&
                                                    mpSeg_.IsUserDictSingleChineseWord(words[i].left->rune))) {
                res.push_back(words[i]);
                continue;
            }

            // if mp Get a single one and it is not in userdict, collect it in sequence
            size_t j = i;

            while (j < words.size() && words[j].left == words[j].right &&
                   !mpSeg_.IsUserDictSingleChineseWord(words[j].left->rune)) {
                j++;
            }

            // Cut the sequence with hmm
            assert(j - 1 >= i);
            // TODO
            hmmSeg_.CutRuneArray(words[i].left, words[j - 1].left + 1, hmmRes);

            //put hmm result to result
            for (size_t k = 0; k < hmmRes.size(); k++) {
                res.push_back(hmmRes[k]);
            }

            //clear tmp vars
            hmmRes.clear();

            //let i jump over this piece
            i = j - 1;
        }
    }

    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<string>& res, bool hmm,
                     size_t) const override {
        //目前hmm默认开启，后期如有需要关闭再修改--jxx20210519
//        if (!hmm) {
//            mpSeg_.CutRuneArray(begin, end, res);
//            return;
//        }
        std::ignore = hmm;
        vector<WordRange> words;
        assert(end >= begin);
        words.reserve(end - begin);
        mpSeg_.CutRuneArray(begin, end, words);

        vector<WordRange> hmmRes;
        hmmRes.reserve(end - begin);

        for (size_t i = 0; i < words.size(); i++) {
            //if mp Get a word, it's ok, put it into result
            if (words[i].left != words[i].right) {
                res.push_back(GetStringFromRunes(s, words[i].left, words[i].right));
                continue;
            }
            if (mpSeg_.IsUserDictSingleChineseWord(words[i].left->rune)
                    || i == (words.size() - 1)) {//i++后如果是最后一个字符则直接push_back
                res.push_back(GetStringFromRunes(s, words[i].left, words[i].right));
                continue;
            }

            // if mp Get a single one and it is not in userdict, collect it in sequence
            size_t j = i + 1; //当前i字符为单独的字符并且不在用户字典里（i字符不是最后一个字符），直接判定j字符

            while (j < (words.size() - 1) && words[j].left == words[j].right &&
                   !mpSeg_.IsUserDictSingleChineseWord(words[j].left->rune)) {
                j++;
            }

            // Cut the sequence with hmm
            assert(j - 1 >= i);
            // TODO
            hmmSeg_.CutRuneArray(words[i].left, words[j - 1].left + 1, hmmRes);

            //put hmm result to result
            for (size_t k = 0; k < hmmRes.size(); k++) {
                res.push_back(GetStringFromRunes(s, hmmRes[k].left, hmmRes[k].right));
            }

            //clear tmp vars
            hmmRes.clear();

            //let i jump over this piece
            i = j - 1;
        }
    }

    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, unordered_map<string, KeyWord>& res, bool hmm,
                     size_t) const override {
        std::ignore = hmm;
        vector<WordRange> words;
        vector<WordRange> hmmRes;
        assert(end >= begin);
        if (3 == begin->len or 4 == begin->len) {
            words.reserve(end - begin);
            mpSeg_.CutRuneArray(begin, end, words);
            hmmRes.reserve(words.size());
        } else {
            hmmRes.reserve(end - begin);
        }

        if (words.size() != 0) {//存在中文分词结果
            for (size_t i = 0; i < words.size(); i++) {

                string str = GetStringFromRunes(s, words[i].left, words[i].right);

                if (words[i].left != words[i].right) {
#ifdef STOP_WORDS_USE_CEDAR_SEGMENT
                    if (0 < stopWords_.exactMatchSearch<int>(str.c_str(), str.size())) {
                        continue;
                    }
#else
                    if (stopWords_.find(str) != stopWords_.end()) {
                        continue;
                    }
#endif
                    res[str].offsets.push_back(words[i].left->offset);
                    res[str].weight += 1.0;
                    continue;
                }

                if (mpSeg_.IsUserDictSingleChineseWord(words[i].left->rune)
                        || i == (words.size() - 1)) {//i++后如果是最后一个字符则直接push_back
#ifdef STOP_WORDS_USE_CEDAR_SEGMENT
                    if (0 < stopWords_.exactMatchSearch<int>(str.c_str(), str.size())) {
                        continue;
                    }
#else
                    if (stopWords_.find(str) != stopWords_.end()) {
                        continue;
                    }
#endif
                    res[str].offsets.push_back(words[i].left->offset);
                    res[str].weight += 1.0;
                    continue;
                }
                // if mp Get a single one and it is not in userdict, collect it in sequence
                size_t j = i + 1; //当前i字符为单独的字符并且不在用户字典里（i字符不是最后一个字符），直接判定j字符
                bool isLastWordsSingle(false);
                while (j <= (words.size() - 1)
                       && words[j].left == words[j].right
                       && !mpSeg_.IsUserDictSingleChineseWord(words[j].left->rune)) {
                    if (j == (words.size() - 1)) {//最后一个分词结果是单字
                        isLastWordsSingle = true;
                        break;
                    }
                    j++;
                }

                // Cut the sequence with hmm
                assert(j - 1 >= i);
                // TODO
                if (isLastWordsSingle) {
                    hmmSeg_.CutRuneArray(words[i].left, words[j].left + 1, hmmRes);
                } else {
                    hmmSeg_.CutRuneArray(words[i].left, words[j].left, hmmRes);
                }

                //put hmm result to result
                for (size_t k = 0; k < hmmRes.size(); k++) {
                    string hmmStr = GetStringFromRunes(s, hmmRes[k].left, hmmRes[k].right);
#ifdef STOP_WORDS_USE_CEDAR_SEGMENT
                    if (0 < stopWords_.exactMatchSearch<int>(hmmStr.c_str(), hmmStr.size())) {
                        continue;
                    }
#else
                    if (/*IsSingleWord(hmmStr) || */stopWords_.find(hmmStr) != stopWords_.end()) {
                        continue;
                    }
#endif

                    res[hmmStr].offsets.push_back(hmmRes[k].left->offset);
                    res[hmmStr].weight += 1.0;
                }

                //clear tmp vars
                hmmRes.clear();

                //let i jump over this piece
                if (isLastWordsSingle) {
                    break;
                }
                i = j - 1;
            }
        } else {//不存在中文分词结果
            for (size_t i = 0; i < (size_t)(end - begin); i++) {
                string str = s.substr((begin+i)->offset, (begin+i)->len);
                res[str].offsets.push_back((begin+i)->offset);
                res[str].weight += 1.0;
            }
        }
    }

    const DictTrie* GetDictTrie() const override {
        return mpSeg_.GetDictTrie();
    }

    bool Tag(const string& src, vector<pair<string, string> >& res) const override {
        return tagger_.Tag(src, res, *this);
    }

    string LookupTag(const string &str) const {
        return tagger_.LookupTag(str, *this);
    }

    void LoadStopWordDict(const string& filePath) {
        ifstream ifs(filePath.c_str());
        if(not ifs.is_open()){
            return ;
        }
        XCHECK(ifs.is_open()) << "open " << filePath << " failed";
        string line ;

        while (getline(ifs, line)) {
#ifdef STOP_WORDS_USE_CEDAR_SEGMENT
            stopWords_.update(line.c_str(), line.size(), 1);
#else
            stopWords_.insert(line);
#endif
        }

        assert(stopWords_.size());
    }
private:
#ifdef STOP_WORDS_USE_CEDAR_SEGMENT
    cedar::da<int, -1, -2, false> stopWords_;
#else
    unordered_set<string> stopWords_;
#endif
    MPSegment mpSeg_;
    HMMSegment hmmSeg_;
    PosTagger tagger_;

}; // class MixSegment

} // namespace cppjieba

