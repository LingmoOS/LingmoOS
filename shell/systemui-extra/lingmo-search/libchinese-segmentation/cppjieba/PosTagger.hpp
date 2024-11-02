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

#include "limonp/StringUtil.hpp"
#include "segment-trie/segment-trie.h"
//#include "DictTrie.hpp"
//#include "SegmentTagged.hpp"

namespace cppjieba {
using namespace limonp;

static const char* const POS_M = "m";
static const char* const POS_ENG = "eng";
static const char* const POS_X = "x";

class PosTagger {
public:
    PosTagger() {
    }
    ~PosTagger() {
    }

    bool Tag(const string& src, vector<pair<string, string> >& res, const SegmentTagged& segment) const {
        vector<string> CutRes;
        segment.CutToStr(src, CutRes);

        for (vector<string>::iterator itr = CutRes.begin(); itr != CutRes.end(); ++itr) {
            res.push_back(make_pair(*itr, LookupTag(*itr, segment)));
        }

        return !res.empty();
    }

    string LookupTag(const string &str, const SegmentTagged& segment) const {
        const DictTrie * dict = segment.GetDictTrie();
        assert(dict != nullptr);
        const auto tmp = dict->Find(str);

        if (tmp == nullptr || tmp->GetTag().empty()) {
            RuneStrArray runes;

            if (!DecodeRunesInString(str, runes)) {
                XLOG(ERROR) << "Decode failed.";
                return POS_X;
            }

            return SpecialRule(runes);
        } else {
            return tmp->GetTag();
        }
    }

private:
    const char* SpecialRule(const RuneStrArray& unicode) const {
        size_t m = 0;
        size_t eng = 0;

        for (size_t i = 0; i < unicode.size() && eng < unicode.size() / 2; i++) {
            if (unicode[i].rune < 0x80) {
                eng ++;

                if ('0' <= unicode[i].rune && unicode[i].rune <= '9') {
                    m++;
                }
            }
        }

        // ascii char is not found
        if (eng == 0) {
            return POS_X;
        }

        // all the ascii is number char
        if (m == eng) {
            return POS_M;
        }

        // the ascii chars contain english letter
        return POS_ENG;
    }

}; // class PosTagger

} // namespace cppjieba

