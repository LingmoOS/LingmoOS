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

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <ostream>
#include "limonp/LocalVector.hpp"
#include "limonp/StringUtil.hpp"
#include "common-struct.h"

namespace cppjieba {

using std::string;
using std::vector;

typedef uint32_t Rune;

inline std::ostream& operator << (std::ostream& os, const Word& w) {
    return os << "{\"word\": \"" << w.word << "\", \"offset\": " << w.offset << "}";
}

struct DatMemElem {
    double weight = 0.0;
    char tag[8] = {};

    void SetTag(const string & str) {
        memset(&tag[0], 0, sizeof(tag));
        strncpy(&tag[0], str.c_str(), std::min(str.size(), sizeof(tag) - 1));
    }

    string GetTag() const {
        return &tag[0];
    }
};

struct DatDag {
    limonp::LocalVector<pair<size_t, const DatMemElem *> > nexts;
    //double max_weight;
    //size_t max_next;
};

struct RuneInfo {
    Rune rune;
    uint32_t offset;
    uint32_t len;
    uint32_t unicode_offset = 0;
    uint32_t unicode_length = 0;
    RuneInfo(): rune(0), offset(0), len(0) {
    }
    RuneInfo(Rune r, uint32_t o, uint32_t l)
        : rune(r), offset(o), len(l) {
    }
    RuneInfo(Rune r, uint32_t o, uint32_t l, uint32_t unicode_offset, uint32_t unicode_length)
        : rune(r), offset(o), len(l), unicode_offset(unicode_offset), unicode_length(unicode_length) {
    }
}; // struct RuneInfo

inline std::ostream& operator << (std::ostream& os, const RuneInfo& r) {
    return os << "{\"rune\": \"" << r.rune << "\", \"offset\": " << r.offset << ", \"len\": " << r.len << "}";
}

typedef limonp::LocalVector<Rune> RuneArray;
typedef limonp::LocalVector<struct RuneInfo> RuneStrArray;

// [left, right]
struct WordRange {
    RuneStrArray::const_iterator left;
    RuneStrArray::const_iterator right;
    WordRange(RuneStrArray::const_iterator l, RuneStrArray::const_iterator r)
        : left(l), right(r) {
    }
    size_t Length() const {
        return right - left;
    }

    bool IsAllAscii() const {
        for (RuneStrArray::const_iterator iter = left; iter <= right; ++iter) {
            if (iter->rune >= 0x80) {
                return false;
            }
        }

        return true;
    }
}; // struct WordRange


inline bool DecodeRunesInString(const string& s, RuneArray& arr) {
    arr.clear();
    return limonp::Utf8ToUnicode32(s, arr);
}

inline RuneArray DecodeRunesInString(const string& s) {
    RuneArray result;
    DecodeRunesInString(s, result);
    return result;
}

inline bool DecodeRunesInString(const string& s, RuneStrArray& runes) {

    uint32_t tmp;
    uint32_t offset = 0;
    runes.clear();
    uint32_t len(0);
    for (size_t i = 0; i < s.size();) {
      if (!(s.data()[i] & 0x80)) { // 0xxxxxxx
        // 7bit, total 7bit
        tmp = (uint8_t)(s.data()[i]) & 0x7f;
        i++;
        len = 1;
      } else if ((uint8_t)s.data()[i] <= 0xdf && i + 1 < s.size()) { // 110xxxxxx
        // 5bit, total 5bit
        tmp = (uint8_t)(s.data()[i]) & 0x1f;

        // 6bit, total 11bit
        tmp <<= 6;
        tmp |= (uint8_t)(s.data()[i+1]) & 0x3f;
        i += 2;
        len = 2;
      } else if((uint8_t)s.data()[i] <= 0xef && i + 2 < s.size()) { // 1110xxxxxx
        // 4bit, total 4bit
        tmp = (uint8_t)(s.data()[i]) & 0x0f;

        // 6bit, total 10bit
        tmp <<= 6;
        tmp |= (uint8_t)(s.data()[i+1]) & 0x3f;

        // 6bit, total 16bit
        tmp <<= 6;
        tmp |= (uint8_t)(s.data()[i+2]) & 0x3f;

        i += 3;
        len = 3;
      } else if((uint8_t)s.data()[i] <= 0xf7 && i + 3 < s.size()) { // 11110xxxx
        // 3bit, total 3bit
        tmp = (uint8_t)(s.data()[i]) & 0x07;

        // 6bit, total 9bit
        tmp <<= 6;
        tmp |= (uint8_t)(s.data()[i+1]) & 0x3f;

        // 6bit, total 15bit
        tmp <<= 6;
        tmp |= (uint8_t)(s.data()[i+2]) & 0x3f;

        // 6bit, total 21bit
        tmp <<= 6;
        tmp |= (uint8_t)(s.data()[i+3]) & 0x3f;

        i += 4;
        len = 4;
      } else {
        return false;
      }
      RuneInfo x(tmp, offset, len, i, 1);
      runes.push_back(x);
      offset += len;
    }
    return true;
}

class RunePtrWrapper {
public:
    const RuneInfo * m_ptr = nullptr;

public:
    explicit RunePtrWrapper(const RuneInfo * p) : m_ptr(p) {}

    uint32_t operator *() {
        return m_ptr->rune;
    }

    RunePtrWrapper operator ++(int) {
        m_ptr ++;
        return RunePtrWrapper(m_ptr);
    }

    bool operator !=(const RunePtrWrapper & b) const {
        return this->m_ptr != b.m_ptr;
    }
};

inline string EncodeRunesToString(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end) {
    string str;
    RunePtrWrapper it_begin(begin), it_end(end);
    limonp::Unicode32ToUtf8(it_begin, it_end, str);
    return str;
}

inline void EncodeRunesToString(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, string& str) {
    RunePtrWrapper it_begin(begin), it_end(end);
    limonp::Unicode32ToUtf8(it_begin, it_end, str);
    return;
}

class Unicode32Counter {
public :
    size_t length = 0;
    void clear() {
        length = 0;
    }
    void push_back(uint32_t) {
        ++length;
    }
};

inline size_t Utf8CharNum(const char * str, size_t length) {
    Unicode32Counter c;

    if (limonp::Utf8ToUnicode32(str, length, c)) {
        return c.length;
    }

    return 0;
}

inline size_t Utf8CharNum(const string & str) {
    return Utf8CharNum(str.data(), str.size());
}

inline bool IsSingleWord(const string& str) {
    return Utf8CharNum(str) == 1;
}


// [left, right]
inline Word GetWordFromRunes(const string& s, RuneStrArray::const_iterator left, RuneStrArray::const_iterator right) {
    assert(right->offset >= left->offset);
    uint32_t len = right->offset - left->offset + right->len;
    uint32_t unicode_length = right->unicode_offset - left->unicode_offset + right->unicode_length;
    return Word(s.substr(left->offset, len), left->offset, left->unicode_offset, unicode_length);
}

inline string GetStringFromRunes(const string& s, RuneStrArray::const_iterator left, RuneStrArray::const_iterator right) {
    assert(right->offset >= left->offset);
    //uint32_t len = right->offset - left->offset + right->len;
    return s.substr(left->offset, right->offset - left->offset + right->len);
}

inline void GetWordsFromWordRanges(const string& s, const vector<WordRange>& wrs, vector<Word>& words) {
    for (size_t i = 0; i < wrs.size(); i++) {
        words.push_back(GetWordFromRunes(s, wrs[i].left, wrs[i].right));
    }
}

inline void GetWordsFromWordRanges(const string& s, const vector<WordRange>& wrs, vector<string>& words) {
    for (size_t i = 0; i < wrs.size(); i++) {
        words.push_back(GetStringFromRunes(s, wrs[i].left, wrs[i].right));
    }
}

inline void GetStringsFromWords(const vector<Word>& words, vector<string>& strs) {
    strs.resize(words.size());

    for (size_t i = 0; i < words.size(); ++i) {
        strs[i] = words[i].word;
    }
}

const size_t MAX_WORD_LENGTH = 512;

} // namespace cppjieba

