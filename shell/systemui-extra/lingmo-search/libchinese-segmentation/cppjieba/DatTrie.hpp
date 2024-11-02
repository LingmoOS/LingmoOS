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
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <QDebug>

#include <algorithm>
#include <utility>

#include "limonp/Md5.hpp"
#include "Unicode.hpp"
//#define USE_DARTS_CLONE
#ifdef USE_DARTS_CLONE
#include "../storage-base/darts-clone/darts.h"
#else
#include "../storage-base/cedar/cedar.h"
#endif

namespace cppjieba {

using std::pair;

struct DatElement {
    string word;
    string tag;
    double weight = 0;

    bool operator < (const DatElement & b) const {
        if (word == b.word) {
            return this->weight > b.weight;
        }

        return this->word < b.word;
    }
};

struct IdfElement {
    string word;
    double idf = 0;

    bool operator < (const IdfElement & b) const {
        if (word == b.word) {
            return this->idf > b.idf;
        }

        return this->word < b.word;
    }
};

struct PinYinElement
{
    string word;
    string tag;

    bool operator < (const DatElement & b) const {
        return this->word < b.word;
    }
};

inline std::ostream & operator << (std::ostream& os, const DatElement & elem) {
    return os << "word=" << elem.word << "/tag=" << elem.tag << "/weight=" << elem.weight;
}

struct PinYinMemElem {
    char tag[6] = {};

    void SetTag(const string & str) {
        memset(&tag[0], 0, sizeof(tag));
        strncpy(&tag[0], str.c_str(), std::min(str.size(), sizeof(tag) - 1));
    }

    string GetTag() const {
        return &tag[0];
    }
};

inline std::ostream & operator << (std::ostream& os, const DatMemElem & elem) {
    return os << "/tag=" << elem.GetTag() << "/weight=" << elem.weight;
}
#ifdef USE_DARTS_CLONE
typedef Darts::DoubleArray JiebaDAT;
#else
typedef cedar::da<int, -1, -2, false> JiebaDAT;
#endif


struct CacheFileHeader {
    char md5_hex[32] = {};
    double min_weight = 0;
    uint32_t elements_num = 0;
    uint32_t dat_size = 0;
};

static_assert(sizeof(DatMemElem) == 16, "DatMemElem length invalid");
static_assert((sizeof(CacheFileHeader) % sizeof(DatMemElem)) == 0, "DatMemElem CacheFileHeader length equal");


class DatTrie {
public:
    DatTrie() {}
    ~DatTrie() {
        ::munmap(mmap_addr_, mmap_length_);
        mmap_addr_ = nullptr;
        mmap_length_ = 0;

        ::close(mmap_fd_);
        mmap_fd_ = -1;
    }

    const DatMemElem * Find(const string & key) const {
#ifdef USE_DARTS_CLONE
        JiebaDAT::result_pair_type find_result;
        dat_.exactMatchSearch(key.c_str(), find_result);

        if ((0 == find_result.length) || (find_result.value < 0) || ((size_t)find_result.value >= elements_num_)) {
            return nullptr;
        }

        return &elements_ptr_[ find_result.value ];
#else
        int result = dat_.exactMatchSearch<int>(key.c_str());
        if (result < 0)
            return nullptr;
        return &elements_ptr_[result];
#endif
    }

    const double Find(const string & key, std::size_t length, std::size_t node_pos) const {
#ifdef USE_DARTS_CLONE
        JiebaDAT::result_pair_type find_result;
        dat_.exactMatchSearch(key.c_str(), find_result, length, node_pos);

        if ((0 == find_result.length) || (find_result.value < 0) || ((size_t)find_result.value >= elements_num_)) {
            return -1;
        }

        return idf_elements_ptr_[ find_result.value ];
#else
        int result = dat_.exactMatchSearch<int>(key.c_str(), length, node_pos);
        if (result < 0)
            return -1;
        return idf_elements_ptr_[result];
#endif
    }

    const PinYinMemElem * PinYinFind(const string & key) const {
#ifdef USE_DARTS_CLONE
        JiebaDAT::result_pair_type find_result;
        dat_.exactMatchSearch(key.c_str(), find_result);

        if ((0 == find_result.length) || (find_result.value < 0) || ((size_t)find_result.value >= elements_num_)) {
            return nullptr;
        }

        return &pinyin_elements_ptr_[ find_result.value ];
#else
        int result = dat_.exactMatchSearch<int>(key.c_str());
        if (result < 0)
            return nullptr;
        return &pinyin_elements_ptr_[result];
#endif
    }

    void Find(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
              vector<struct DatDag>&res, size_t max_word_len) const {

        res.clear();
        res.resize(end - begin);

        string text_str;
        EncodeRunesToString(begin, end, text_str);

        static const size_t max_num = 128;
        JiebaDAT::result_pair_type result_pairs[max_num] = {};

        for (size_t i = 0, begin_pos = 0; i < size_t(end - begin); i++) {

            std::size_t num_results = dat_.commonPrefixSearch(&text_str[begin_pos], &result_pairs[0], max_num);

            res[i].nexts.push_back(pair<size_t, const DatMemElem *>(i + 1, nullptr));

            for (std::size_t idx = 0; idx < num_results; ++idx) {
                auto & match = result_pairs[idx];

                if ((match.value < 0) || ((size_t)match.value >= elements_num_)) {
                    continue;
                }

                auto const char_num = Utf8CharNum(&text_str[begin_pos], match.length);

                if (char_num > max_word_len) {
                    continue;
                }

                auto pValue = &elements_ptr_[match.value];

                if (1 == char_num) {
                    res[i].nexts[0].second = pValue;
                    continue;
                }

                res[i].nexts.push_back(pair<size_t, const DatMemElem *>(i + char_num, pValue));
            }

            begin_pos += limonp::UnicodeToUtf8Bytes((begin + i)->rune);
        }
    }

    /*
    void Find_Reverse(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
              vector<struct DatDag>&res, size_t max_word_len) const {

        res.clear();
        res.resize(end - begin);

        string text_str;
        EncodeRunesToString(begin, end, text_str);

        static const size_t max_num = 128;
        JiebaDAT::result_pair_type result_pairs[max_num] = {};

        size_t str_size = end - begin;
        for (size_t i = 0, begin_pos = text_str.size(); i < str_size; i++) {

            begin_pos -= (end - i - 1)->len;
            std::size_t num_results = dat_.commonPrefixSearch(&text_str[begin_pos], &result_pairs[0], max_num);
            res[str_size - i - 1].nexts.push_back(pair<size_t, const DatMemElem *>(str_size - i, nullptr));

            for (std::size_t idx = 0; idx < num_results; ++idx) {
                auto & match = result_pairs[idx];
                if ((match.value < 0) || ((size_t)match.value >= elements_num_)) {
                    continue;
                }

                auto const char_num = Utf8CharNum(&text_str[begin_pos], match.length);

                if (char_num > max_word_len) {
                    continue;
                }

                auto pValue = &elements_ptr_[match.value];

                if (1 == char_num) {
                    res[str_size - i - 1].nexts[0].second = pValue;
                    continue;
                }

                res[str_size - i - 1].nexts.push_back(pair<size_t, const DatMemElem *>(str_size - 1 - i + char_num, pValue));
            }
        }
    }*/

    void Find(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end,
              vector<WordRange>& words, size_t max_word_len) const {

        string text_str;
        EncodeRunesToString(begin, end, text_str);

        static const size_t max_num = 128;
        JiebaDAT::result_pair_type result_pairs[max_num] = {};//存放字典查询结果
        size_t str_size = end - begin;
        double max_weight[str_size];//存放逆向路径最大weight
        for (size_t i = 0; i<str_size; i++) {
            max_weight[i] = -3.14e+100;
        }
        int max_next[str_size];//存放动态规划后的分词结果
        //memset(max_next,-1,str_size);

        double val(0);
        for (size_t i = 0, begin_pos = text_str.size(); i < str_size; i++) {
            size_t nextPos = str_size - i;//逆向计算
            begin_pos -= (end - i - 1)->len;

            std::size_t num_results = dat_.commonPrefixSearch(&text_str[begin_pos], &result_pairs[0], max_num);
            if (0 == num_results) {//字典不存在则单独分词
                val = min_weight_;

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
                    if ((match.value < 0) || ((size_t)match.value >= elements_num_)) {
                        continue;
                    }
                    auto const char_num = Utf8CharNum(&text_str[begin_pos], match.length);
                    if (char_num > max_word_len) {
                        continue;
                    }
                    auto pValue = &elements_ptr_[match.value];

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
    double GetMinWeight() const {
        return min_weight_;
    }

    void SetMinWeight(double d) {
        min_weight_ = d ;
    }

    bool InitBuildDat(vector<DatElement>& elements, const string & dat_cache_file, const string & md5) {
        BuildDatCache(elements, dat_cache_file, md5);
        return InitAttachDat(dat_cache_file, md5);
    }

    bool InitBuildDat(vector<IdfElement>& elements, const string & dat_cache_file, const string & md5) {
        BuildDatCache(elements, dat_cache_file, md5);
        return InitIdfAttachDat(dat_cache_file, md5);
    }

    bool InitBuildDat(vector<PinYinElement>& elements, const string & dat_cache_file, const string & md5) {
        BuildDatCache(elements, dat_cache_file, md5);
        return InitPinYinAttachDat(dat_cache_file, md5);
    }

    bool InitAttachDat(const string & dat_cache_file, const string & md5) {
        mmap_fd_ = ::open(dat_cache_file.c_str(), O_RDONLY);

        if (mmap_fd_ < 0) {
            return false;
        }

        const auto seek_off = ::lseek(mmap_fd_, 0, SEEK_END);
        assert(seek_off >= 0);
        mmap_length_ = seek_off;

        mmap_addr_ = reinterpret_cast<char *>(mmap(NULL, mmap_length_, PROT_READ, MAP_SHARED, mmap_fd_, 0));
        assert(MAP_FAILED != mmap_addr_);

        assert(mmap_length_ >= sizeof(CacheFileHeader));
        CacheFileHeader & header = *reinterpret_cast<CacheFileHeader*>(mmap_addr_);
        elements_num_ = header.elements_num;
        min_weight_ = header.min_weight;
        assert(sizeof(header.md5_hex) == md5.size());

        if (0 != memcmp(&header.md5_hex[0], md5.c_str(), md5.size())) {
            return false;
        }

        assert(mmap_length_ == sizeof(header) + header.elements_num * sizeof(DatMemElem)  + header.dat_size * dat_.unit_size());
        elements_ptr_ = (const DatMemElem *)(mmap_addr_ + sizeof(header));
        char * dat_ptr = mmap_addr_ + sizeof(header) + sizeof(DatMemElem) * elements_num_;
        dat_.set_array(dat_ptr, header.dat_size);
        return true;
    }

    bool InitIdfAttachDat(const string & dat_cache_file, const string & md5) {
        mmap_fd_ = ::open(dat_cache_file.c_str(), O_RDONLY);

        if (mmap_fd_ < 0) {
            return false;
        }

        const auto seek_off = ::lseek(mmap_fd_, 0, SEEK_END);
        assert(seek_off >= 0);
        mmap_length_ = seek_off;

        mmap_addr_ = reinterpret_cast<char *>(mmap(NULL, mmap_length_, PROT_READ, MAP_SHARED, mmap_fd_, 0));
        assert(MAP_FAILED != mmap_addr_);

        assert(mmap_length_ >= sizeof(CacheFileHeader));
        CacheFileHeader & header = *reinterpret_cast<CacheFileHeader*>(mmap_addr_);
        elements_num_ = header.elements_num;
        min_weight_ = header.min_weight;
        assert(sizeof(header.md5_hex) == md5.size());

        if (0 != memcmp(&header.md5_hex[0], md5.c_str(), md5.size())) {
            return false;
        }

        assert(mmap_length_ == sizeof(header) + header.elements_num * sizeof(double)  + header.dat_size * dat_.unit_size());
        idf_elements_ptr_ = (const double *)(mmap_addr_ + sizeof(header));
        char * dat_ptr = mmap_addr_ + sizeof(header) + sizeof(double) * elements_num_;
        dat_.set_array(dat_ptr, header.dat_size);
        return true;
    }

    bool InitPinYinAttachDat(const string & dat_cache_file, const string & md5) {
        mmap_fd_ = ::open(dat_cache_file.c_str(), O_RDONLY);

        if (mmap_fd_ < 0) {
            return false;
        }

        const auto seek_off = ::lseek(mmap_fd_, 0, SEEK_END);
        assert(seek_off >= 0);
        mmap_length_ = seek_off;

        mmap_addr_ = reinterpret_cast<char *>(mmap(NULL, mmap_length_, PROT_READ, MAP_SHARED, mmap_fd_, 0));
        assert(MAP_FAILED != mmap_addr_);

        assert(mmap_length_ >= sizeof(CacheFileHeader));
        CacheFileHeader & header = *reinterpret_cast<CacheFileHeader*>(mmap_addr_);
        elements_num_ = header.elements_num;
        min_weight_ = header.min_weight;
        assert(sizeof(header.md5_hex) == md5.size());

        if (0 != memcmp(&header.md5_hex[0], md5.c_str(), md5.size())) {
            return false;
        }

        assert(mmap_length_ == sizeof(header) + header.elements_num * sizeof(PinYinMemElem)  + header.dat_size * dat_.unit_size());
        pinyin_elements_ptr_ = (const PinYinMemElem *)(mmap_addr_ + sizeof(header));
        char * dat_ptr = mmap_addr_ + sizeof(header) + sizeof(PinYinMemElem) * elements_num_;
        dat_.set_array(dat_ptr, header.dat_size);
        return true;
    }

private:
    void BuildDatCache(vector<DatElement>& elements, const string & dat_cache_file, const string & md5) {
        std::sort(elements.begin(), elements.end());

        vector<const char*> keys_ptr_vec;
        vector<int> values_vec;
        vector<DatMemElem> mem_elem_vec;

        keys_ptr_vec.reserve(elements.size());
        values_vec.reserve(elements.size());
        mem_elem_vec.reserve(elements.size());

        CacheFileHeader header;
        header.min_weight = min_weight_;
        assert(sizeof(header.md5_hex) == md5.size());
        memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

        for (size_t i = 0; i < elements.size(); ++i) {
            keys_ptr_vec.push_back(elements[i].word.data());
            values_vec.push_back(i);
            mem_elem_vec.push_back(DatMemElem());
            auto & mem_elem = mem_elem_vec.back();
            mem_elem.weight = elements[i].weight;
            mem_elem.SetTag(elements[i].tag);
        }

        auto const ret = dat_.build(keys_ptr_vec.size(), &keys_ptr_vec[0], NULL, &values_vec[0]);
        assert(0 == ret);
        header.elements_num = mem_elem_vec.size();
        header.dat_size = dat_.size();

        {
            string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
            ::umask(S_IWGRP | S_IWOTH);
            //const int fd =::mkstemp(&tmp_filepath[0]);
            const int fd =::mkstemp((char *)tmp_filepath.data());
            qDebug() << "mkstemp :" << errno << tmp_filepath.data();
            assert(fd >= 0);
            ::fchmod(fd, 0644);

            auto write_bytes = ::write(fd, (const char *)&header, sizeof(header));
            write_bytes += ::write(fd, (const char *)&mem_elem_vec[0], sizeof(mem_elem_vec[0]) * mem_elem_vec.size());
            write_bytes += ::write(fd, dat_.array(), dat_.total_size());

            assert(write_bytes == sizeof(header) + mem_elem_vec.size() * sizeof(mem_elem_vec[0]) + dat_.total_size());
            ::close(fd);

            const auto rename_ret = ::rename(tmp_filepath.c_str(), dat_cache_file.c_str());
            assert(0 == rename_ret);
        }
    }

    void BuildDatCache(vector<IdfElement>& elements, const string & dat_cache_file, const string & md5) {
        std::sort(elements.begin(), elements.end());

        vector<const char*> keys_ptr_vec;
        vector<int> values_vec;
        vector<double> mem_elem_vec;

        keys_ptr_vec.reserve(elements.size());
        values_vec.reserve(elements.size());
        mem_elem_vec.reserve(elements.size());

        CacheFileHeader header;
        header.min_weight = min_weight_;
        assert(sizeof(header.md5_hex) == md5.size());
        memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

        for (size_t i = 0; i < elements.size(); ++i) {
            keys_ptr_vec.push_back(elements[i].word.data());
            values_vec.push_back(i);
            mem_elem_vec.push_back(elements[i].idf);
        }

        auto const ret = dat_.build(keys_ptr_vec.size(), &keys_ptr_vec[0], NULL, &values_vec[0]);
        assert(0 == ret);
        header.elements_num = mem_elem_vec.size();
        header.dat_size = dat_.size();

        {
            string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
            ::umask(S_IWGRP | S_IWOTH);
            //const int fd =::mkstemp(&tmp_filepath[0]);
            const int fd =::mkstemp((char *)tmp_filepath.data());
            qDebug() << "mkstemp error:" << errno << tmp_filepath.data();
            assert(fd >= 0);
            ::fchmod(fd, 0644);

            auto write_bytes = ::write(fd, (const char *)&header, sizeof(header));
            write_bytes += ::write(fd, (const char *)&mem_elem_vec[0], sizeof(double) * mem_elem_vec.size());
            write_bytes += ::write(fd, dat_.array(), dat_.total_size());

            assert(write_bytes == sizeof(header) + mem_elem_vec.size() * sizeof(double) + dat_.total_size());
            ::close(fd);

            const auto rename_ret = ::rename(tmp_filepath.c_str(), dat_cache_file.c_str());
            assert(0 == rename_ret);
        }
    }

    void BuildDatCache(vector<PinYinElement>& elements, const string & dat_cache_file, const string & md5) {
        //std::sort(elements.begin(), elements.end());

        vector<const char*> keys_ptr_vec;
        vector<int> values_vec;
        vector<PinYinMemElem> mem_elem_vec;

        keys_ptr_vec.reserve(elements.size());
        values_vec.reserve(elements.size());
        mem_elem_vec.reserve(elements.size());

        CacheFileHeader header;
        header.min_weight = min_weight_;
        assert(sizeof(header.md5_hex) == md5.size());
        memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

        for (size_t i = 0; i < elements.size(); ++i) {
            keys_ptr_vec.push_back(elements[i].word.data());
            values_vec.push_back(i);
            mem_elem_vec.push_back(PinYinMemElem());
            auto & mem_elem = mem_elem_vec.back();
            mem_elem.SetTag(elements[i].tag);
        }

        auto const ret = dat_.build(keys_ptr_vec.size(), &keys_ptr_vec[0], NULL, &values_vec[0]);
        assert(0 == ret);
        header.elements_num = mem_elem_vec.size();
        header.dat_size = dat_.size();

        {
            string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
            ::umask(S_IWGRP | S_IWOTH);
            //const int fd =::mkstemp(&tmp_filepath[0]);
            const int fd =::mkstemp((char *)tmp_filepath.data());
            qDebug() << "mkstemp :" << errno << tmp_filepath.data();
            assert(fd >= 0);
            ::fchmod(fd, 0644);

            auto write_bytes = ::write(fd, (const char *)&header, sizeof(header));
            write_bytes += ::write(fd, (const char *)&mem_elem_vec[0], sizeof(mem_elem_vec[0]) * mem_elem_vec.size());
            write_bytes += ::write(fd, dat_.array(), dat_.total_size());

            assert(write_bytes == sizeof(header) + mem_elem_vec.size() * sizeof(mem_elem_vec[0]) + dat_.total_size());
            ::close(fd);

            const auto rename_ret = ::rename(tmp_filepath.c_str(), dat_cache_file.c_str());
            assert(0 == rename_ret);
        }
    }

    DatTrie(const DatTrie &);
    DatTrie &operator=(const DatTrie &);

private:
    JiebaDAT dat_;
    const DatMemElem * elements_ptr_ = nullptr;
    const double * idf_elements_ptr_ = nullptr;
    const PinYinMemElem * pinyin_elements_ptr_ = nullptr;
    size_t elements_num_ = 0;
    double min_weight_ = 0;

    int mmap_fd_ = -1;
    size_t mmap_length_ = 0;
    char * mmap_addr_ = nullptr;
};


inline string CalcFileListMD5(const string & files_list, size_t & file_size_sum) {
    limonp::MD5 md5;

    const auto files = limonp::Split(files_list, "|;");
    file_size_sum = 0;

    for (auto const & local_path : files) {
        const int fd = ::open(local_path.c_str(), O_RDONLY);
        if( fd < 0){
            continue;
        }
        auto const len = ::lseek(fd, 0, SEEK_END);
        if (len > 0) {
            void * addr = ::mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
            assert(MAP_FAILED != addr);

            md5.Update((unsigned char *) addr, len);
            file_size_sum += len;

            ::munmap(addr, len);
        }
        ::close(fd);
    }

    md5.Final();
    return string(md5.digestChars);
}

}
