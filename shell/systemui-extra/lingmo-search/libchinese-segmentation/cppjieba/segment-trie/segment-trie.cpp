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
#include <cmath>
#include "segment-trie.h"

DictTrie::DictTrie(const vector<string> file_paths, string dat_cache_path)
    : StorageBase<DatMemElem, false, DictCacheFileHeader>(file_paths, dat_cache_path)
{
    this->Init();
}

DictTrie::DictTrie(const string &dict_path, const string &user_dict_paths, const string &dat_cache_path)
    : StorageBase<DatMemElem, false, DictCacheFileHeader>(vector<string>{dict_path, user_dict_paths}, dat_cache_path)
{
    this->Init();
}

void DictTrie::LoadSourceFile(const string &dat_cache_file, const string &md5)
{
    DictCacheFileHeader header;
    assert(sizeof(header.md5_hex) == md5.size());
    memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

    int offset(0), elements_num(0), write_bytes(0), data_trie_size(0);
    string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
    umask(S_IWGRP | S_IWOTH);
    const int fd =mkstemp((char *)tmp_filepath.data());
    assert(fd >= 0);
    fchmod(fd, 0644);

    write_bytes = write(fd, (const char *)&header, sizeof(DictCacheFileHeader));

    this->PreLoad();
    this->LoadDefaultDict(fd, write_bytes, offset, elements_num);
    this->LoadUserDict(fd, write_bytes, offset, elements_num);

    write_bytes += write(fd, this->GetDataTrieArray(), this->GetDataTrieTotalSize());

    lseek(fd, sizeof(header.md5_hex), SEEK_SET);
    write(fd, &elements_num, sizeof(int));
    write(fd, &offset, sizeof(int));
    data_trie_size = this->GetDataTrieSize();
    write(fd, &data_trie_size, sizeof(int));
    write(fd, &m_min_weight, sizeof(double));

    close(fd);
    assert((size_t)write_bytes == sizeof(DictCacheFileHeader) + offset + this->GetDataTrieTotalSize());

    tryRename(tmp_filepath, dat_cache_file);
}

const DatMemElem * DictTrie::Find(const string &key) const
{
    int result = this->ExactMatchSearch(key.c_str(), key.size());
    if (result < 0)
        return nullptr;
    return &this->GetElementPtr()[result];
}



void DictTrie::FindDatDag(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<DatDag> &res, size_t max_word_len) const {

    res.clear();
    res.resize(end - begin);

    string text_str;
    EncodeRunesToString(begin, end, text_str);

    static const size_t max_num = 128;
    result_pair_type result_pairs[max_num] = {};

    for (size_t i = 0, begin_pos = 0; i < size_t(end - begin); i++) {

        std::size_t num_results = this->CommonPrefixSearch(&text_str[begin_pos], &result_pairs[0], max_num);

        res[i].nexts.push_back(pair<size_t, const DatMemElem *>(i + 1, nullptr));

        for (std::size_t idx = 0; idx < num_results; ++idx) {
            auto & match = result_pairs[idx];

            if ((match.value < 0) || ((size_t)match.value >= this->GetCacheFileHeaderPtr()->elements_size)) {
                continue;
            }

            auto const char_num = Utf8CharNum(&text_str[begin_pos], match.length);

            if (char_num > max_word_len) {
                continue;
            }

            const DatMemElem * pValue = &this->GetElementPtr()[match.value];

            if (1 == char_num) {
                res[i].nexts[0].second = pValue;
                continue;
            }

            res[i].nexts.push_back(pair<size_t, const DatMemElem *>(i + char_num, pValue));
        }

        begin_pos += limonp::UnicodeToUtf8Bytes((begin + i)->rune);
    }
}

void DictTrie::FindWordRange(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange> &words, size_t max_word_len) const {

    string text_str;
    EncodeRunesToString(begin, end, text_str);

    static const size_t max_num = 128;
    result_pair_type result_pairs[max_num] = {};//存放字典查询结果
    size_t str_size = end - begin;
    double max_weight[str_size];//存放逆向路径最大weight
    for (size_t i = 0; i<str_size; i++) {
        max_weight[i] = -3.14e+100;
    }
    size_t max_next[str_size];//存放动态规划后的分词结果
    //memset(max_next,-1,str_size*sizeof(size_t));

    double val(0);
    for (size_t i = 0, begin_pos = text_str.size(); i < str_size; i++) {
        size_t nextPos = str_size - i;//逆向计算
        begin_pos -= (end - i - 1)->len;

        std::size_t num_results = this->CommonPrefixSearch(&text_str[begin_pos], &result_pairs[0], max_num);
        if (0 == num_results) {//字典不存在则单独分词
            val = GetMinWeight();
            if (nextPos  < str_size) {
                val += max_weight[nextPos];
            }
            if ((nextPos <= str_size) && (val > max_weight[nextPos - 1])) {
                max_weight[nextPos - 1] = val;
                max_next[nextPos - 1] = nextPos;
            }
        } else {//字典存在则根据查询结果数量计算最大概率路径
            for (std::size_t idx = 0; idx < num_results; ++idx) {
                auto & match = result_pairs[idx];
                if ((match.value < 0) || ((uint32_t)match.value >= this->GetCacheFileHeaderPtr()->elements_size)) {
                    continue;
                }
                auto const char_num = Utf8CharNum(&text_str[begin_pos], match.length);
                if (char_num > max_word_len) {
                    continue;
                }
                auto * pValue = &this->GetElementPtr()[match.value];

                val = pValue->weight;
                if (1 == char_num) {
                    if (nextPos  < str_size) {
                        val += max_weight[nextPos];
                    }
                    if ((nextPos <= str_size) && (val > max_weight[nextPos - 1])) {
                        max_weight[nextPos - 1] = val;
                        max_next[nextPos - 1] = nextPos;
                    }
                } else {
                    if (nextPos - 1 + char_num  < str_size) {
                        val += max_weight[nextPos - 1 + char_num];
                    }
                    if ((nextPos - 1 + char_num <= str_size) && (val > max_weight[nextPos - 1])) {
                        max_weight[nextPos - 1] = val;
                        max_next[nextPos - 1] = nextPos - 1 + char_num;
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < str_size;) {//统计动态规划结果
        assert(max_next[i] > i);
        assert(max_next[i] <= str_size);
        WordRange wr(begin + i, begin + max_next[i] - 1);
        words.push_back(wr);
        i = max_next[i];
    }
}

bool DictTrie::IsUserDictSingleChineseWord(const Rune &word) const {
    return IsIn(m_user_dict_single_chinese_word, word);
}

void DictTrie::PreLoad()
{
    ifstream ifs(DICT_PATH);
    string line;
    vector<string> buf;

    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#") or line.empty()) {
            continue;
        }
        limonp::Split(line, buf, " ");
        if (buf.size() != 3)
            continue;
        m_freq_sum += atof(buf[1].c_str());
    }
}

void DictTrie::LoadDefaultDict(const int &fd, int &write_bytes, int &offset, int &elements_num)
{
    ifstream ifs(DICT_PATH);
    string line;
    vector<string> buf;

    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#") or line.empty()) {
            continue;
        }
        limonp::Split(line, buf, " ");
        if (buf.size() != 3)
            continue;
        DatMemElem node_info;
        node_info.weight = log(atof(buf[1].c_str()) / m_freq_sum);
        node_info.SetTag(buf[2]);
        this->Update(buf[0].c_str(), buf[0].size(), elements_num);
        offset += (sizeof(DatMemElem));
        elements_num++;
        if (m_min_weight > node_info.weight) {
            m_min_weight = node_info.weight;
        }
        write_bytes += write(fd, &node_info, sizeof(DatMemElem));
    }
}

void DictTrie::LoadUserDict(const int &fd, int &write_bytes, int &offset, int &elements_num)
{
    ifstream ifs(USER_DICT_PATH);
    string line;
    vector<string> buf;
    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#") or line.empty()) {
            continue;
        }
        limonp::Split(line, buf, " ");
        if (buf.size() != 3)
            continue;
        DatMemElem node_info;
        assert(m_freq_sum > 0.0);
        const int freq = atoi(buf[1].c_str());
        node_info.weight = log(1.0 * freq / m_freq_sum);
        node_info.SetTag(buf[2]);
        this->Update(buf[0].c_str(), buf[0].size(), elements_num);
        offset += (sizeof(DatMemElem));
        elements_num++;
        write_bytes += write(fd, &node_info, sizeof(DatMemElem));
        if (Utf8CharNum(buf[0]) == 1) {
            RuneArray word;
            if (DecodeRunesInString(buf[0], word)) {
                m_user_dict_single_chinese_word.insert(word[0]);
            }
        }
    }
}

inline double DictTrie::GetMinWeight() const
{
    return this->GetCacheFileHeaderPtr()->min_weight;
}
