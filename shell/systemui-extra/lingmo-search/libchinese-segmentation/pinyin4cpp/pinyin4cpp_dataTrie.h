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
#ifndef PINYIN4cpp_DATATRIE_H
#define PINYIN4cpp_DATATRIE_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <QDebug>
#include "Md5.hpp"
#include "LocalVector.hpp"
#include "StringUtil.hpp"
//#define USE_REDUCED_TRIE
#include "../storage-base/cedar/cedar.h"
#include "../storage-base/darts-clone/darts.h"

using namespace std;
using std::pair;

struct CacheFileHeader { //todo 字节对齐
    char md5_hex[32] = {};
    uint32_t elements_num = 0;
    uint32_t elements_size = 0;
    uint32_t dat_size = 0;
};

class Pinyin4cppDataTrie {
public:
    Pinyin4cppDataTrie();
    ~Pinyin4cppDataTrie();

    string Find(const string & key) const;

    bool InitBuildDat(map<string, string>& elements, const string & dat_cache_file, const string & md5);

    bool InitAttachDat(const string & dat_cache_file, const string & md5);

private:
    void BuildDatCache(map<string, string>& elements, const string & dat_cache_file, const string & md5);

    Pinyin4cppDataTrie(const Pinyin4cppDataTrie &);
    Pinyin4cppDataTrie &operator=(const Pinyin4cppDataTrie &);

private:
    Darts::DoubleArray m_DoubleArrayDataTrie;
    //cedar::da<int, -1, -2, true> m_DoubleArrayDataTrie;
    const char * m_elementsPtr = nullptr;
    size_t m_elementsNum = 0;
    size_t m_elementsSize = 0;
    size_t m_mmapLength = 0;

    int    m_mmapFd = -1;
    char * m_mmapAddr = nullptr;
};

#endif //PINYIN4cpp_DATATRIE_H
