/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */
#ifndef COMMONSTRUCT_H
#define COMMONSTRUCT_H

#include <string>
#include <vector>

using namespace std;

/**
 * @brief The KeyWord struct
 *
 * @property word the content of keyword
 * @property offsets the Unicode offsets, can be used to check the word pos in a sentence
 * @property weight the weight of the keyword
 */

struct KeyWord {
    string word;
    vector<size_t> offsets;
    double weight;
    ~KeyWord() {
        word = std::move("");
        offsets.clear();
        offsets.shrink_to_fit();
    }
};

/**
 * @brief The Word struct
 *
 * @property word the content of word
 * @property offset the offset of the word(absolute pos, Chinese 3 , English 1)， can be used to check the word pos in a sentence
 * @property unicode_offset the Unicode offset of the word
 * @property unicode_length the Unicode length of the word
 */
struct Word {
    string word;
    uint32_t offset;
    uint32_t unicode_offset;
    uint32_t unicode_length;
    Word(const string& w, uint32_t o)
        : word(w), offset(o) {
    }
    Word(const string& w, uint32_t o, uint32_t unicode_offset, uint32_t unicode_length)
        : word(w), offset(o), unicode_offset(unicode_offset), unicode_length(unicode_length) {
    }
    ~Word() {
        word = std::move("");
    }
}; // struct Word

#endif // COMMONSTRUCT_H
