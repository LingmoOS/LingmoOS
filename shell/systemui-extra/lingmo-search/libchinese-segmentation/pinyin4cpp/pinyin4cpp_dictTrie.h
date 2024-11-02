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
 * Authors: jixiaoxu <jixiaoxu@kylinos.cn>
 *
 */
#ifndef PINYIN4cpp_DICTTRIE_H
#define PINYIN4cpp_DICTTRIE_H

#include "pinyin4cpp_dataTrie.h"
using namespace std;

const size_t SINGLE_WORD_DICT_COLUMN_NUM = 3;
const size_t WORDS_DICT_COLUMN_NUM = 2;

class Pinyin4cppDictTrie {
public:
    Pinyin4cppDictTrie(const string& single_word_dict_path, const string& words_dict_paths, const string & dat_cache_path = "");

    ~Pinyin4cppDictTrie() {}

    string Find(const string &word) const;

    bool Contains(string &word);
    bool IsMultiTone(const string &word);

    size_t GetTotalDictSize() const;

private:
    void Init(const string& single_word_dict_path, const string& words_dict_paths, string dat_cache_path);

    void LoadSingleWordDict(const string& filePath);

    void LoadWordsDict(const string& filePath);

private:
    map<string, string> m_StaticNodeInfos;

    size_t m_TotalDictSize_ = 0;
    Pinyin4cppDataTrie m_DataTrie;

};

inline string CalcFileListMD5(const string & files_list, size_t & file_size_sum);

#endif //PINYIN4cpp_DICTTRIE_H
