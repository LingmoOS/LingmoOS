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
#include <unordered_set>
#include "Unicode.hpp"

namespace cppjieba {

class PreFilter {
public:
    PreFilter(const std::unordered_set<Rune>& symbols,
              const string& sentence)
        : symbols_(symbols) {
        if (!DecodeRunesInString(sentence, sentence_)) {
            XLOG(ERROR) << "decode failed. "<<sentence;
        }

        cursor_ = sentence_.begin();
    }
    ~PreFilter() {
    }
    bool HasNext() const {
        return cursor_ != sentence_.end();
    }
    bool Next(WordRange& wordRange) {

        if (cursor_ == sentence_.end()) {
            return false;
        }

        wordRange.left = cursor_;

        while (cursor_->rune == 0x20 && cursor_ != sentence_.end()) {
            cursor_++;
        }

        if (cursor_ == sentence_.end()) {
            wordRange.right = cursor_;
            return true;
        }

        while (++cursor_ != sentence_.end()) {
            if (cursor_->rune == 0x20) {
                wordRange.right = cursor_;
                return true;
            }
        }

        wordRange.right = sentence_.end();
        return true;
    }

    bool Next(WordRange& wordRange, bool& isNull) {
        isNull = false;
        if (cursor_ == sentence_.end()) {
            return false;
        }

        wordRange.left = cursor_;
        if (cursor_->rune == 0x20) {
            while (cursor_ != sentence_.end()) {
                if (cursor_->rune != 0x20) {
                    if (wordRange.left == cursor_) {
                        cursor_ ++;
                    }
                    wordRange.right = cursor_;
                    isNull = true;
                    return true;
                }
                cursor_ ++;
            }
            return false;
        }

        int max_num = 0;
        uint32_t utf8_num = cursor_->len;

        while (cursor_ != sentence_.end()) {
            if (cursor_->rune == 0x20) {
                if (wordRange.left == cursor_) {
                    cursor_ ++;
                }

                wordRange.right = cursor_;
                return true;
            }

            cursor_ ++;
            max_num++;
            if (max_num >= 1024 or cursor_->len != utf8_num) { //todo 防止一次性传入过多字节，暂定限制为1024个字
                wordRange.right = cursor_;
                return true;
            }
        }

        wordRange.right = sentence_.end();
        return true;
    }

    WordRange Next() {
        WordRange range(cursor_, cursor_);

        while (cursor_ != sentence_.end()) {
            //if (IsIn(symbols_, cursor_->rune)) {
            if (cursor_->rune == 0x20) {
                if (range.left == cursor_) {
                    cursor_ ++;
                }

                range.right = cursor_;
                return range;
            }

            cursor_ ++;
        }

        range.right = sentence_.end();
        return range;
    }
private:
    RuneStrArray::const_iterator cursor_;
    RuneStrArray sentence_;
    const std::unordered_set<Rune>& symbols_;
}; // class PreFilter

} // namespace cppjieba

