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
#ifndef SegmentTrie_H
#define SegmentTrie_H

#include "storage-base.hpp"
#include "cppjieba/Unicode.hpp"

using namespace cppjieba;

const char * const DICT_PATH = DICT_INSTALL_PATH"/jieba.dict.utf8";
const char * const USER_DICT_PATH = DICT_INSTALL_PATH"/user.dict.utf8";

struct DictCacheFileHeader : CacheFileHeaderBase
{
    double min_weight = 0;
};

class DictTrie : public StorageBase<DatMemElem, false, DictCacheFileHeader>
{
public:
    DictTrie(const vector<string> file_paths, string dat_cache_path = "");
    DictTrie(const string& dict_path, const string& user_dict_paths = "", const string & dat_cache_path = "");
    void LoadSourceFile(const string &dat_cache_file, const string &md5) override;

    const DatMemElem *Find(const string &key) const;
    void FindDatDag(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
              vector<struct DatDag>&res, size_t max_word_len = MAX_WORD_LENGTH) const;
    void FindWordRange(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
              vector<WordRange>& words, size_t max_word_len = MAX_WORD_LENGTH) const;
    bool IsUserDictSingleChineseWord(const Rune& word) const;

private:
    DictTrie();
    void PreLoad();
    void LoadDefaultDict(const int &fd, int &write_bytes, int &offset, int &elements_num);
    void LoadUserDict(const int &fd, int &write_bytes, int &offset, int &elements_num);
    double GetMinWeight() const;

    double m_freq_sum = 0.0;
    double m_min_weight = 3.14e+100;
    unordered_set<Rune> m_user_dict_single_chinese_word;
};

#endif // SegmentTrie_H
